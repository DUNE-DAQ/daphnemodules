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
DaphneController::init(const data_t& /* structured args */)
{
}

void
DaphneController::get_info(opmonlib::InfoCollector& ci, int /* level */)
{

  static const std::regex volt_regex(".* VBIAS0= ([^ ]+) VBIAS1= ([^ ]+) VBIAS2= ([^ ]+) VBIAS3= ([^ ]+) VBIAS4= ([^ ]+) POWER.-5v.= ([^ ]+) POWER..2.5v.= ([^ ]+) POWER..CE.= ([^ ]+) TEMP.Celsius.= ([^ ]+) .*");

  static const std::regex current_regex(".* Voltage.mV.= ([^ ]+) .*");

  
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

  for ( int i = 1; i < string_values.size(); ++i ) {
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

  auto ip_string = conf_as_cpp.daphne_address;

  static std::regex ip_regex("[0-9]+.[0-9]+.[0-9]+.([0-9]+)");

  std::smatch matches; 
  
  if ( ! std::regex_match( ip_string, matches, ip_regex) ) {
    throw InvalidIPAddress(ERS_HERE, ip_string);
  }

  auto last = std::stoi(matches[1]);
  m_slot = last % 100;
  if ( m_slot >= 16 ) {
    // Set the slot to the last part of the IP addreess (from 104 to 113)
    // the slot used laster in the code is a 4 bit register, so we need to check we are not overflowing
    throw InvalidSlot(ERS_HERE, m_slot, ip_string);
  }

  TLOG() << "Using daphne at " << ip_string << " with slot " << m_slot; 
  
  m_interface.reset( new  DaphneInterface( ip_string.c_str(), 2001) );

  // channel configuration
  // there is another variable we use to configure each channel the TRIM control:
  // the command to drive it is: 'WR TRIM CH <id_channel> <value>'
  // and value should be in the range 0 - 4095
  // BIASCTRL is configured for the entired board and the max value is 4095
  auto v_biasctrl = conf_as_cpp.biasctrl;

  if ( c.conf.v_biasctrl > 4095 ) 
      throw InvalidBiasCtrlConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);
 
  auto channel_conf = conf_as_cpp.channels;

  for ( const auto & c : channel_conf ) {
    //CH TRIM maximum is 4095
    if ( c.conf.trim > 4095 ) 
      throw InvalidChannelConfiguration(ERS_HERE, c.id, c.conf.trim, c.conf.offset, c.conf.gain);

    //CH OFFSET maximum is 2700 if GAIN is 1, 1500 if GAIN is 2
    if ( c.conf.gain != 1 && c.conf.gain != 2 ) {
      throw InvalidChannelConfiguration(ERS_HERE, c.id, c.conf.trim, c.conf.offset, c.conf.gain);
    }
    if ( c.conf.gain == 1 ) {
      if ( c.conf.offset > 2700 ) 
	throw InvalidChannelConfiguration(ERS_HERE, c.id, c.conf.trim, c.conf.offset, c.conf.gain);
    } else if ( c.conf.gain == 2 ) {
      if ( c.conf.offset > 1500 ) 
	throw InvalidChannelConfiguration(ERS_HERE, c.id, c.conf.trim, c.conf.offset, c.conf.gain);
    }

    if ( c.id >= DaphneController::s_max_channels ) {
      throw InvalidChannelConfiguration(ERS_HERE, c.id, c.conf.trim, c.conf.offset, c.conf.gain);
    }
           
    m_channel_confs[c.id] = c.conf;
  }

  // afe configuration
  auto afe_conf = conf_as_cpp.afes;

  for ( const auto & afe : afe_conf ) {
    // an afe only serves 8 channels.
    // Since channels can be disabled (offset = 0) we first check if at least one of the channel of the AFE
    // is enabled, otherwise the corresponding AFE is left in its default configuration, which is the disabled

    // the channel depend on the id of the AFE, so first we check if the AFE ID is valid
    // added v_bias controls the bias per AFE, and shares similar properties to v_gain
    // max value this variable can take in hardware is 80V 
    // max value it should take in configuration is 1500DAC ~ 55V


    if ( afe.id >= DaphneController::s_max_afes ) 
      throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, 
				afe.conf.reg_4, afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);

    bool used = false;
    for ( auto ch = afe.id * 8 ; ch < (afe.id+1)*8 ; ++ch ) {
      if ( m_channel_confs[ch].offset > 0 ) {
	used = true;
	break;
      }
    }

    if ( used ) {
      if ( afe.conf.reg_52 >= 65536 )
	// this is a 16 bit register
        throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);

      if ( afe.conf.reg_4 >= 32 )
	// this is a 5 bit register
        throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias;

      if ( afe.conf.reg_51 >= 16384 )
	// this is a 14 bit register
        throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);

      if ( afe.conf.v_gain >= 4096 )
	// this is a 12 bit register
        throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);

      if ( afe.conf.v_bias >= 1500 )
	// this is a 12 bit register
        throw InvalidAFEConfiguration(ERS_HERE, afe.id, afe.conf.reg_52, afe.conf.reg_4, 
			afe.conf.reg_51, afe.conf.v_gain, afe.conf.v_bias);
      m_afe_confs[afe.id] = afe.conf;
    }
  }
  
  configure_timing_endpoints();
  
  configure_analog_chain();
  
  align_DDR();
  
  
  // ---------------------------------------------
  // set self trigger or full stream
  // for full stream
  // thing.write(0x3001, {0xaa}); 
  // thing.write(0x6001, {0b00000000});   // for safety
  // check
  // thing.read(0x3001, 1)
  // the result should be 0xaa

  // self_trigger
  //  thing.write(0x3001, {0x3});
  //  thing.write(0x6000, {700});  // threshold, to be configured 14 bits from configuration
  // thing.write(0x6001, {0b11111111});   // this is a 40 bit regsiter, so I could construct this number from the enabled channels
  // check 
  // thing.read(0x3001, 1)
  // the result should be 0x3
  // 


  // In the case of of full stream, we can only stream data from 16 channels at maximum
  // This is because of badnwidth.
  // So if we run in fullstream we need to set which channes to stream out
  // We need to assign each channel to a link (streamSender). We have 4 links for each board.
  // write in register 0x500X X in 0 to F the channel that is supposed to be streamed out.
  // X is the identifier of a output physically in 4 cables
  // The channles are not identified with an id from 0-39, they have a different identifier to represent the
  // cables in the fron of the board. They are grouped in 8 
  // Conf ch -> DAQ ch
  // 0-7     -> 0-7
  // 8-15    -> 10-17
  // 16-23   -> 20-27
  // 24-31   -> 30-37
  // 32-39   -> 40-47

  // the configuration of which link should be as first come first serve.
  // to a maximum of 16 channels
  
  // we get a list of 
  // Let's say I want to see 0x5001
  //                         0x5004 10
  // To be discussed - channel/link sorting
  // -----------------------------------------
  // thing.write_reg(0x2000, {1234});         
  // 








  
  auto res = m_interface->read_register(0x9000, 1);
  for ( auto v : res ) {
    TLOG() << v;
  }
  
  res = m_interface->read_buffer(0x40000000,15);
  for ( auto v : res ) {
    TLOG() << v;
  }

  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << "Board configured in " << duration.count() << " microseconds";
  
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
  auto register_check = m_interface->read_register(0x4000, 1);
  check = std::bitset<16>(register_check[0]);
  if ( ! check[12] ) {
    throw TimingEndpointNotReady(ERS_HERE, check.to_string() );
  }

  TLOG() << "Done donfiguring timing endpoint";

}

void DaphneController::configure_analog_chain() {

  TLOG() << "Configuring analog chain";
  
  auto result = m_interface->send_command("CFG AFE ALL INITIAL");
  TLOG() << result.command << " -> " << result.result;

  auto result = m_interface->send_command("WR VBIASCTRL V " + std::to_string(v_biasctrl));
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
    
  }

  //   // to check if the configuration went throguh we can
  //   //cmd (thing, "RD OFFSET CH " + std::to_string(ch), true);
  //   // But Manuel said that this is not necessary to be done all the time

  // Reg 51 will not configure the clamp level, we default it to 1XX becuase we are in Low noise mode
  // and we alwaus want to disable the clamp

  // Reg 51 exceptions
  // - [4:0] set to 0
  // - [5]   set to 0
  // - [7:6] set to 0
  // - [8]   set to 0
  
  for ( size_t afe = 0; afe < m_afe_confs.size() ; ++afe) {
    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 52 V " + std::to_string(m_afe_confs[afe].reg_52) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 4 V " + std::to_string(m_afe_confs[afe].reg_4) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " REG 51 V " + std::to_string(m_afe_confs[afe].reg_51) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR AFE " + std::to_string(afe) + " VGAIN V " + std::to_string(m_afe_confs[afe].v_gain) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(
	"WR BIASSET AFE " + std::to_string(afe) + " V " + std::to_string(m_afe_confs[afe].v_bias) );
    TLOG() << result.command << " -> " << result.result;
  }
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


} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
