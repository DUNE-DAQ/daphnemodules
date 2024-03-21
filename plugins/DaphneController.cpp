/**
 * @file DaphneController.cpp
 *
 * Implementations of DaphneController's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DaphneController.hpp"

#include "daphnemodules/daphnecontroller/Nljs.hpp"
#include "daphnemodules/daphnecontrollerinfo/InfoNljs.hpp"

#include <string>
#include <logging/Logging.hpp>

#include <regex>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <bitset>
#include <thread>

namespace dunedaq::daphnemodules {

DaphneController::DaphneController(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &DaphneController::do_conf);
}


void
DaphneController::get_info(opmonlib::InfoCollector& ci, int /* level */)
{

  static const std::regex volt_regex(".* VBIAS0= ([^ ]+) VBIAS1= ([^ ]+) VBIAS2= ([^ ]+) VBIAS3= ([^ ]+) VBIAS4= ([^ ]+) POWER.-5v.= ([^ ]+) POWER..2.5v.= ([^ ]+) POWER..CE.= ([^ ]+) TEMP.Celsius.= ([^ ]+) .*");

  // this lock is not completely necessary because of the internal locks in the interface
  // but it's a safety measure to make sure that this does not interfere with complex operations
  const std::lock_guard<std::mutex> lock(m_mutex);
  
  if ( ! m_interface ) return ;
  
  auto cmd_res = m_interface->send_command("RD VM ALL");
  TLOG() << cmd_res.result ;
  
  std::smatch string_values; 
						 
  if ( ! std::regex_match( cmd_res.result, string_values, volt_regex ) ) {
    ers::error( WrongMonitoringString(ERS_HERE, cmd_res.result) );
    return ;
  }
    
  daphnecontrollerinfo::VoltageInfo v_info;

  std::vector<double> values(string_values.size());

  for ( size_t i = 1; i < string_values.size(); ++i ) {
    try {
      values[i] = std::stod( string_values[i] );
    }  catch ( const std::logic_error & e) {
      ers::error( FailedStringConversion(ERS_HERE, string_values[i], e) );
      return;
    }
  }
    
  v_info.v_bias_0 = values[1];
  v_info.v_bias_1 = values[2];
  v_info.v_bias_2 = values[3];
  v_info.v_bias_3 = values[4];
  v_info.v_bias_4 = values[5];
  
  v_info.power_minus5v = values[6];
  v_info.power_plus2p5v = values[7];
  v_info.power_ce = values[8];
  
  v_info.temperature = values[9];
  
  ci.add(v_info);

  // //current monitor
  // for ( size_t ch = 0; ch < m_channel_confs.size() ; ++ch ) {
  //   if ( m_channel_confs[ch].offset > 0 ) {
  //     auto current_res = m_interface->send_command("RD CM CH " + std::to_string(ch) );
  //     TLOG() << current_res.command << " -> " << current_res.result ; 
  //   }
    

  // // monitor of the ADC
  // m_interface->write_register(0x2000, {1234});
  // // this trigger the spy buffers
  
  // // read 100 values for each channel, register ch 8 of each afe,   by looping on all the afe we use
  // for ( size_t afe = 0; afe < m_afe_confs.size() ; ++afe ) {
  //   if ( m_afe_confs[afe].v_gain > 0 ) {
  //     for ( size_t ch = 0; ch < m_channel_confs.size() ; ++ch ) {
  // 	if ( m_channel_confs[ch].offset > 0 ) {
  // 	  auto data = m_interface->read_register(0x40000000 + (afe * 0x100000) + (ch * 0x10000), 50);  // first 50
  // 	  // data[0]

  // 	  data = m_interface->read_register(0x40000000 + (afe * 0x100000) + (ch * 0x10000) +50, 50);  // second 50

  // 	}

      
      
  //   }
  // }



  
}

void
DaphneController::do_conf(const data_t& conf_as_json)
{
  auto start_time = std::chrono::high_resolution_clock::now();
  
  auto conf_as_cpp = conf_as_json.get<daphnecontroller::Conf>();

  // during configuration no other operations are allowed
  const std::lock_guard<std::mutex> lock(m_mutex);
  
  create_interface(conf_as_cpp.daphne_address);

  validate_configuration(conf_as_cpp);
  
  configure_timing_endpoints();
  
  configure_analog_chain();
  
  align_DDR();
  
  configure_trigger_mode();
  
  // we get a list of 
  // Let's say I want to see 0x5001
  //                         0x5004 10
  // To be discussed - channel/link sorting
  // -----------------------------------------
  // thing.write_reg(0x2000, {1234});         
  // 


  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << "Board configured in " << duration.count() << " microseconds";
  
}


void
DaphneController::create_interface(const std::string & ip) {

  static std::regex ip_regex("[0-9]+.[0-9]+.[0-9]+.([0-9]+)");
  
  std::smatch matches; 
  
  if ( ! std::regex_match( ip, matches, ip_regex) ) {
    throw InvalidIPAddress(ERS_HERE, ip);
  }

  auto last = std::stoi(matches[1]);
  m_slot = last % 100;
  if ( m_slot >= 16 ) {
    // Set the slot to the last part of the IP addreess (from 104 to 113)
    // the slot used laster in the code is a 4 bit register, so we need to check we are not overflowing
    throw InvalidSlot(ERS_HERE, m_slot, ip);
  }

  TLOG() << "Using daphne at " << ip << " with slot " << (int)m_slot; 

  m_interface.reset( new  DaphneInterface( ip.c_str(), 2001) );
  
}

void
DaphneController::validate_configuration(const daphnecontroller::Conf & c) {

  // channel configuration
  // there is another variable we use to configure each channel the TRIM control:
  // the command to drive it is: 'WR TRIM CH <id_channel> <value>'
  // and value should be in the range 0 - 4095
  
  // BIASCTRL is configured for the entired board and the max value is 4095
  if ( c.biasctrl > 4095 ) 
    throw InvalidBiasControl(ERS_HERE, c.biasctrl);

  m_bias_ctrl = c.biasctrl;
 
  const auto & channel_conf = c.channels;

  for ( const auto & ch : channel_conf ) {

    if ( ch.id >= DaphneController::s_max_channels ) {
      throw InvalidChannelId(ERS_HERE, ch.id, DaphneController::s_max_channels);
    }
    
    //CH TRIM maximum is 4095
    if ( ch.conf.trim > 4095 ) 
      throw InvalidChannelConfiguration(ERS_HERE, ch.id, ch.conf.trim, ch.conf.offset, ch.conf.gain);

    //CH OFFSET maximum is 2700 if GAIN is 1, 1500 if GAIN is 2
    if ( ch.conf.gain != 1 && ch.conf.gain != 2 ) {
      throw InvalidChannelConfiguration(ERS_HERE, ch.id, ch.conf.trim, ch.conf.offset, ch.conf.gain);
    }
    if ( ch.conf.gain == 1 ) {
      if ( ch.conf.offset > 2700 ) 
	throw InvalidChannelConfiguration(ERS_HERE, ch.id, ch.conf.trim, ch.conf.offset, ch.conf.gain);
    } else if ( ch.conf.gain == 2 ) {
      if ( ch.conf.offset > 1500 ) 
	throw InvalidChannelConfiguration(ERS_HERE, ch.id, ch.conf.trim, ch.conf.offset, ch.conf.gain);
    }
           
    m_channel_confs[ch.id] = ch.conf;
  }

  // afe configuration
  const auto & afe_conf = c.afes;

  for ( const auto & afe : afe_conf ) {
    // an afe only serves 8 channels.
    // Since channels can be disabled (offset = 0) we first check if at least one of the channel of the AFE
    // is enabled, otherwise the corresponding AFE is left in its default configuration, which is the disabled

    // the channel depend on the id of the AFE, so first we check if the AFE ID is valid
    // added v_bias controls the bias per AFE, and shares similar properties to v_gain
    // max value this variable can take in hardware is 80V 
    // max value it should take in configuration is 1500DAC ~ 55V


    if ( afe.id >= DaphneController::s_max_afes ) 
      throw InvalidChannelId( ERS_HERE, afe.id, DaphneController::s_max_afes);

    bool used = false;
    for ( auto ch = afe.id * 8 ; ch < (afe.id+1)*8 ; ++ch ) {
      if ( channel_used(ch) ) {
	used = true;
	break;
      }
    }

    if ( used ) {

      AFEConf afe_conf;
      
      if ( afe.v_gain >= 4096 )
	// this is a 12 bit register
        throw InvalidAFEVoltage(ERS_HERE, afe.id, afe.v_gain, afe.v_bias);

      afe_conf.v_gain = afe.v_gain;
      
      if ( afe.v_bias >= 1500 )
	// this is a 12 bit register
	// but the bias have to be under a certain value to operate in cold temperature
	// The maximum value depends on the brand of the SiPM, but it's always smaller than 1500 anyway
	throw InvalidAFEVoltage(ERS_HERE, afe.id, afe.v_gain, afe.v_bias);

      afe_conf.v_bias = afe.v_bias;

      // ADC, reg 4 has no parsing as it's all made of booleans
      std::bitset<5> reg4;
      // bits 0 and 2 are reserved
      reg4[1] = afe.adc.resolution;
      reg4[3] = afe.adc.output_format;
      reg4[4] = afe.adc.SB_first;

      afe_conf.reg4 = (decltype(afe_conf.reg4)) reg4.to_ulong() ;

      // PGA, reg 51
      std::bitset<14> reg51(afe.pga.lpf_cut_frequency);
      if ( afe.pga.lpf_cut_frequency != 0 && afe.pga.lpf_cut_frequency != 2 && afe.pga.lpf_cut_frequency != 4 )
	throw InvalidPGAConf(ERS_HERE, afe.id, afe.pga.lpf_cut_frequency);
      
      reg51 <<= 1;
      reg51[4] = afe.pga.integrator_disable;
      reg51[7] = true;  // clamp is always disabled and we are in low noise mode
      reg51[13] = afe.pga.gain;
      
      afe_conf.reg51 = (decltype(afe_conf.reg51)) reg51.to_ulong() ;

      // LNA, reg52
      std::bitset<16> reg52;
      if ( afe.lna.clamp > 3 ) // only 4 options allowed
	throw InvalidLNAConf(ERS_HERE, afe.id, afe.lna.clamp, afe.lna.gain);
	
      decltype(reg52) clamp(afe.lna.clamp);
      clamp <<= 6;

      reg52[12] = afe.lna.integrator_disable;

      if ( afe.lna.gain > 2 )  // only 3 options allowed
	throw InvalidLNAConf(ERS_HERE, afe.id, afe.lna.clamp, afe.lna.gain);

      decltype(reg52) gain(afe.lna.gain);
      clamp <<= 13;

      reg52 |= clamp;
      reg52 |= gain;

      afe_conf.reg52 = (decltype(afe_conf.reg52)) reg52.to_ulong() ;

      m_afe_confs[afe.id] = afe_conf;
    }
  }  // loop over the AFE

  // configuring the trigger mode
  if ( c.self_trigger_threshold > 16383 )
    // this is a 14 bit register
    InvalidThreshold(ERS_HERE, c.self_trigger_threshold);

  m_self_threshold = c.self_trigger_threshold;

  if ( m_self_threshold == 0 ) {
    // we need to set the list of channels to broadcast
    for ( const auto & ch : c.full_stream_channels ) {
      if ( ch >= DaphneController::s_max_channels ) 
	throw InvalidChannelId(ERS_HERE, ch, DaphneController::s_max_channels);

      m_full_stream_channels.push_back(ch);

      if (m_full_stream_channels.size()>16) {
	// we can only stream 16 channels at most
	throw TooManyChannels( ERS_HERE, c.full_stream_channels.size() );
      }
    }
  }
  
}

 

void
DaphneController::configure_timing_endpoints() {

  TLOG() << "Configuring timing endpoint";
  m_interface->write_register(0x4001, {0x1});
  m_interface->write_register(0x3000, {0x002081 + uint64_t(0x400000 * m_slot)});
  m_interface->write_register(0x4003, {1234});

  // waiting for the PLL to lock
  std::bitset<16> check;
  int counter = 0;
  do {
    ++counter;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto register_check = m_interface->read_register(0x4000, 1);
    check = std::bitset<16>(register_check[0]);
    if ( counter > 200 ) break;
  } while (!check[0]);

  if ( ! check[0] ) {
    throw PLLNotLocked(ERS_HERE, "MMCM0");
  }
  
  m_interface->write_buffer(0x4002, {1234});

  // waiting for the PLL to lock
  counter = 0;
  do {
    ++counter;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto register_check = m_interface->read_register(0x4000, 1);
    check = std::bitset<16>(register_check[0]);
    if ( counter > 200 ) break;
  } while (!check[1]);

  if ( ! check[1] ) {
    throw PLLNotLocked(ERS_HERE, "MMCM1");
  }
  
  // at this point everything that is in register 0x4000 is the status of the timing endpoint
  // we need to check bit 12 to check if the timing endpoint is valid
  // 0 = not ok
  // 1 = ok
  // should things fail, we can print a lot of messages from register 0x4000
  
  // there's a necessary delay to let DAPHNE receive and compare the timestamp
  // like previous cases, we are going to try to cut this by checking if the system is ready every 5 ms
  counter = 0;
  do {
    ++counter;
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    auto register_check = m_interface->read_register(0x4000, 1);
    check = std::bitset<16>(register_check[0]);
    if ( counter > 500 ) break;
    // we ae happy to wait up to a second (5ms * 200) until calling an error
  } while (!check[12]);

  if ( ! check[12] ) {
    throw TimingEndpointNotReady(ERS_HERE, check.to_string() );
  }
  
  TLOG() << "Done donfiguring timing endpoint";

}

void DaphneController::configure_analog_chain() {

  TLOG() << "Configuring analog chain";
  
  auto result = m_interface->send_command("CFG AFE ALL INITIAL");
  TLOG() << result.command << " -> " << result.result;

  result = m_interface->send_command("WR VBIASCTRL V " + std::to_string(m_bias_ctrl));
  TLOG() << result.command << " -> " << result.result;
  
  for ( size_t ch = 0; ch < m_channel_confs.size() ; ++ch ) {

    result = m_interface->send_command(
      "WR TRIM CH " + std::to_string(ch) + " V " + std::to_string(m_channel_confs[ch].trim) );
    TLOG() << result.command << " -> " << result.result;
    
    result = m_interface->send_command(
      "WR OFFSET CH " + std::to_string(ch) + " V " + std::to_string(m_channel_confs[ch].offset) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
      "CFG OFFSET CH " + std::to_string(ch) + " GAIN " + std::to_string(m_channel_confs[ch].gain) );
    TLOG() << result.command << " -> " << result.result;

    // the defaul values of offset and gain are set so that they correspond to the setting to disable the channel
    // hence, this loop does both the job of enabling and disebling
    
  } // channel loop

  //   // to check if the configuration went throguh we can
  //   //cmd (thing, "RD OFFSET CH " + std::to_string(ch), true);
  //   // But Manuel said that this is not necessary to be done all the time

  for ( size_t afe = 0; afe < m_afe_confs.size() ; ++afe) {
    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 52 V " + std::to_string(m_afe_confs[afe].reg52) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 4 V " + std::to_string(m_afe_confs[afe].reg4) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 51 V " + std::to_string(m_afe_confs[afe].reg51) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " VGAIN V " + std::to_string(m_afe_confs[afe].v_gain) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR BIASSET AFE " + std::to_string(afe) + " V " + std::to_string(m_afe_confs[afe].v_bias) );
    TLOG() << result.command << " -> " << result.result;

  } // afe loop
  
  //   // To check these values we can do things like
  //   // cmd (thing, "RD AFE " + std::to_string(AFE) + " REG 52", true);
  //   // for all these registers and get the values from the replies

}


void DaphneController::align_DDR() {

  TLOG() << "Aligning DDR";
  
  m_interface->write_register(0x2001, {1234});
  m_interface->write_register(0x2001, {1234});
  m_interface->write_register(0x2001, {1234});
  // this is correct to be done 3 times

  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  // this is necessary to give time to the board to align the AFE DDR
  // Otherwise further checks become pointless

  // --------------------------------------------
  // checking if the alignement is achieved
  // --------------------------------------------
  m_interface->write_register(0x2000, {1234});
  // this trigger the spy buffers
    
  // read register ch 8 of each afe,   by looping on all the afe we use
  for ( size_t afe = 0; afe < m_afe_confs.size() ; ++afe ) {
    if ( m_afe_confs[afe].v_gain > 0 ) {
      auto data = m_interface->read_register(0x40000000 + (afe * 0x100000) + (8 * 0x10000), 15);  // ch = 8

      // things are ok when the data is 0x3f80
      if ( data[0] != DaphneController::s_frame_alignment_good ) 
	throw DDRNotAligned(ERS_HERE, afe, data[0] );
    }
  }
  
}


void
DaphneController::configure_trigger_mode() {

  if ( m_self_threshold > 0 ) {
    // se are in self trigger mode
    m_interface->write_register(0x3001, {0x3});  // only link0 is enabled
    m_interface->write_register(0x6000, {m_self_threshold});

    std::bitset<DaphneController::s_max_channels> mask;
    // we unmask all the channels that are enabled
    for ( ChannelId ch = 0; ch < m_channel_confs.size(); ++ch ) {
      if ( channel_used(ch) )
	mask[ch] = true;
    }
    m_interface->write_register(0x6001, {(uint64_t)mask.to_ulong()});

    // check 
    // thing.read(0x3001, 1)
    // the result should be 0x3

  } else {
    m_interface->write_register(0x3001, {0xaa});
    m_interface->write_register(0x6000, {0});  // for safety we mask everything

    size_t stream_id = 0;
    for ( const auto & ch : m_full_stream_channels ) {

      // The channles are not identified with an id from 0-39, they have a different identifier to represent the
      // cables in the fron of the board. They are grouped in 8 
      // Conf ch -> DAQ ch
      // 0-7     -> 0-7
      // 8-15    -> 10-17
      // 16-23   -> 20-27
      // 24-31   -> 30-37
      // 32-39   -> 40-47

      auto reg = 0x5000 + stream_id; // stream is first come first served basis
      auto value = (ch/8)*10 + ch%8;

      m_interface->write_register(reg, {(uint64_t)value});

      ++stream_id;
    }
    // check 
    // thing.read(0x3001, 1)
    // the result should be 0xaa
    // 
  }

}
  
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
