/**
 * @file DaphneV2ControllerModule.cpp
 *
 * Implementations of DaphneV2ControllerModule's functions
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DaphneV2ControllerModule.hpp"
#include "appmodel/DaphneV2BoardConf.hpp"
#include "appmodel/DaphneV2Channel.hpp"
#include "appmodel/DaphneV2AFE.hpp"
#include "appmodel/DaphneV2ADC.hpp"
#include "appmodel/DaphneV2LNA.hpp"
#include "appmodel/DaphneV2PGA.hpp"
#include "appmodel/DaphneConf.hpp"

#include <string>
#include <logging/Logging.hpp>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <regex>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <bitset>
#include <thread>
#include <algorithm>
#include <fmt/format.h>


namespace dunedaq::daphnemodules {

DaphneV2ControllerModule::DaphneV2ControllerModule(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &DaphneV2ControllerModule::do_conf);
  register_command("scrap", &DaphneV2ControllerModule::do_scrap);
  //  register_command("dump_buffers", &DaphneV2ControllerModule::dump_buffers);
}


void
DaphneV2ControllerModule::init(std::shared_ptr<appfwk::ModuleConfiguration> mcfg) {

  auto mdal = mcfg->module<conf_t>(get_name());
  if (!mdal) {
    throw ConfigurationFailed(ERS_HERE, get_name());
  }
  m_module_config = mdal;
}
  

  
void
DaphneV2ControllerModule::generate_opmon_data()
{

  if ( ! m_interface ) return ;
  
  if ( m_scrap_called.load() ) return;

  // read the channel counters
  constexpr uint64_t s_dropped_counter_address = 0x40700000;
  constexpr uint64_t s_start_counter_buffer = 0x40800000;
  constexpr auto  s_packets_counter_address = s_start_counter_buffer + s_max_channels*8;
  constexpr auto  s_tot_packets_counter_address = s_packets_counter_address + s_max_channels*8;

  // this lock is not completely necessary because of the internal locks in the interface
  // but it's a safety measure to make sure that this does not interfere with complex operations
  const std::lock_guard<std::mutex> lock(m_mutex);
    
  try {
  
    opmon::StreamInfo stream_info;    
    
    // read total packages sent to felix
    auto tot_pack_buf = m_interface->read_register(s_tot_packets_counter_address, 1);
    stream_info.set_total_packets(tot_pack_buf[0]);
    if ( m_last_package_counter.load() != 0 ) {
      stream_info.set_new_packets(stream_info.total_packets() - m_last_package_counter.exchange(stream_info.total_packets()));
    } else {
      m_last_package_counter = stream_info.total_packets();
    }
    
  // read total packages not sent to felix
    auto tot_dropped_buf = m_interface->read_register(s_dropped_counter_address, 1);
    stream_info.set_total_dropped_packets(tot_dropped_buf[0]);
    if ( m_last_unsent_counter.load() != 0 ) {
      stream_info.set_new_dropped_packets(stream_info.total_dropped_packets() - m_last_unsent_counter.exchange(stream_info.total_dropped_packets()));
    } else {
      m_last_unsent_counter = stream_info.total_dropped_packets();
    }
    
    publish( std::move(stream_info) );
  } catch ( const ers::Issue & e ) {
    ers::warning( MonitoringFailed(ERS_HERE, "data stream", e));
  }

  
  for ( ChannelId c = 0; c < s_max_channels; ++c ) {
    
    try { 
      opmon::ChannelInfo c_info;
      
      auto trig_buf = m_interface->read_register(s_start_counter_buffer+c*8, 1);  
      const auto & trig = trig_buf[0];
      c_info.set_total_triggers(trig);
      if ( m_channel_counters[c].triggers.load() != 0 ) {
	c_info.set_new_triggers(trig - m_channel_counters[c].triggers.exchange(trig));
      } else {
	m_channel_counters[c].triggers = trig;
      }
      
      auto pack_buf = m_interface->read_register(s_packets_counter_address+c*8, 1);
      const auto & pack = pack_buf[0];
      c_info.set_total_packets(pack);
      if ( m_channel_counters[c].packets.load() != 0 ) {
	c_info.set_new_packets(pack - m_channel_counters[c].packets.exchange(pack));
      } else {
	m_channel_counters[c].packets = pack;
      }

      publish( std::move(c_info), { {"channel", fmt::format("{}", c)} } );

    } catch ( const ers::Issue & e) {
      ers::warning( MonitoringFailed(ERS_HERE, fmt::format("Channel {}", c), e));
    }
    
  } 
    
  // gatehring the rest of the information
  static const std::regex volt_regex(".* VBIAS0= ([^ ]+) VBIAS1= ([^ ]+) VBIAS2= ([^ ]+) VBIAS3= ([^ ]+) VBIAS4= ([^ ]+) POWER.-5v.= ([^ ]+) POWER..2.5v.= ([^ ]+) POWER..CE.= ([^ ]+) TEMP.Celsius.= ([^ ]+) .*");


  try {

    opmon::GeneralInfo v_info;
    
    auto cmd_res = m_interface->send_command("RD VM ALL");
    
    std::smatch string_values; 
    
    if ( ! std::regex_match( cmd_res.result, string_values, volt_regex ) ) {
      ++m_error_counter;
      WrongMonitoringString temp_error(ERS_HERE,
				       m_module_config -> get_slot(), m_error_counter, cmd_res.result);
      TLOG() << temp_error;
      if ( m_error_counter >= 10 ) {
	ers::error( temp_error );
	return;
      }
      if ( m_error_counter >= 5 ) {
	ers::warning( temp_error );
	return;
      }
      return ;
    }
    
    //reset the error counter
    m_error_counter = 0;
    
    std::vector<double> values(string_values.size());
    
    for ( size_t i = 1; i < string_values.size(); ++i ) {
      try {
	values[i] = std::stod( string_values[i] );
      }  catch ( const std::logic_error & e) {
	ers::error( FailedStringConversion(ERS_HERE, string_values[i], e) );
	return;
      }
    }
    
    v_info.set_v_bias_0(values[1]);
    v_info.set_v_bias_1(values[2]);
    v_info.set_v_bias_2(values[3]);
    v_info.set_v_bias_3(values[4]);
    v_info.set_v_bias_4(values[5]);
    
    v_info.set_power_minus5v(values[6]);
    v_info.set_power_plus2p5v(values[7]);
    v_info.set_power_ce(values[8]);
    
    v_info.set_temperature(values[9]);

    publish( std::move(v_info) );

  } catch (const ers::Issue & e ){
    ers::warning(MonitoringFailed(ERS_HERE, "general info", e));
  }

  
  
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
DaphneV2ControllerModule::do_conf(const data_t&)
{
  auto start_time = std::chrono::high_resolution_clock::now();

  auto slot = m_module_config->get_slot();
  if ( slot >= 16 ) 
    //   // the slot used laster in the code is a 4 bit register, so we need to check we are not overflowing
    throw InvalidSlot(ERS_HERE, slot, m_module_config->get_address());
  
  // during configuration no other operations are allowed
  const std::lock_guard<std::mutex> lock(m_mutex);
  
  create_interface(m_module_config->get_address(),
		   m_module_config->get_daphne_conf()->get_timeout());

  validate_configuration( * m_module_config->get_board_conf() );
  
  configure_timing_endpoints();
  
  configure_analog_chain(true);
  
  align_DDR();
  
  configure_trigger_mode();
  
  // we get a list of 
  // Let's say I want to see 0x5001
  //                         0x5004 10
  // To be discussed - channel/link sorting
  // -----------------------------------------
  // thing.write_reg(0x2000, {1234});         
  // 

  m_scrap_called = false;

  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << get_name() << ": board configured in " << duration.count() << " microseconds";
  
}


void
DaphneV2ControllerModule::do_scrap(const data_t&)
{
  auto start_time = std::chrono::high_resolution_clock::now();

  m_scrap_called = true;
  
  // during configuration no other operations are allowed
  const std::lock_guard<std::mutex> lock(m_mutex);

  configure_analog_chain(false);

  // break the interface
  m_interface.release();

  auto end_time = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << get_name() << ": board releasd in " << duration.count() << " microseconds";
  
}

  

void
DaphneV2ControllerModule::create_interface(const std::string & ip, std::chrono::milliseconds timeout) {

  static std::regex ip_regex("[0-9]+.[0-9]+.[0-9]+.([0-9]+)");
  
  std::smatch matches; 
  
  if ( ! std::regex_match( ip, matches, ip_regex) ) {
    throw InvalidIPAddress(ERS_HERE, ip);
  }

  TLOG() << get_name() << ": using daphne at " << ip << " with slot " << (int)m_module_config->get_slot(); 

  m_interface.reset( new  DaphneInterface( ip.c_str(), 2001, timeout ) );
  
}

void
DaphneV2ControllerModule::validate_configuration(const appmodel::DaphneV2BoardConf & c) const {

  const auto & channel_confs = c.get_active_channels();

  for ( const auto & ch : channel_confs ) {
    auto id = ch->get_channel_id();

    //CH OFFSET maximum is 2700 if GAIN is 1, 1500 if GAIN is 2
    auto gain = ch->get_gain();
    if ( gain != 1 && gain != 2 ) {
      throw InvalidChannelConfiguration(ERS_HERE,
					id, ch->get_trim(), ch->get_offset(), gain);
    }
    auto offset = ch -> get_offset();
    if ( gain == 1 ) {
      if ( offset > 2700 ) 
 	throw InvalidChannelConfiguration(ERS_HERE, id, ch->get_trim(), offset, gain);
    } else if ( gain == 2 ) {
      if ( offset > 1500 ) 
 	throw InvalidChannelConfiguration(ERS_HERE, id, ch->get_trim(), offset, gain);
    }
  } // loop over channels

  auto size = c.get_full_stream_channels().size();
  if (size>16) {
    // we can only stream 16 channels at most
    throw TooManyChannels( ERS_HERE, size );
  }
}

void
DaphneV2ControllerModule::configure_timing_endpoints() {

  TLOG() << get_name() << ": configuring timing endpoint";
  m_interface->write_register(0x4001, {0x1});
  m_interface->write_register(0x3000, {0x002081 + uint64_t(0x400000 * m_module_config->get_slot())});
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
    throw PLLNotLocked(ERS_HERE, m_module_config->get_slot(), "MMCM0");
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
    throw PLLNotLocked(ERS_HERE, m_module_config->get_slot(), "MMCM1");
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
    throw TimingEndpointNotReady(ERS_HERE, m_module_config->get_slot(), check.to_string() );
  }
  
  TLOG() << get_name() << ": done donfiguring timing endpoint";

}

void DaphneV2ControllerModule::configure_analog_chain(bool initial_config) {

  TLOG() << get_name() << ": configuring analog chain";

  if (initial_config) {
    auto result = m_interface->send_command("CFG AFE ALL INITIAL");
    TLOG() << result.command << " -> " << result.result;
  }

  auto board_conf = initial_config ? m_module_config->get_board_conf() :
    m_module_config->get_daphne_conf()->get_default_v2_settings();
    
  auto result = m_interface->send_command(fmt::format("WR VBIASCTRL V {}", board_conf->get_bias_ctrl()));
  TLOG() << result.command << " -> " << result.result;
  
  for ( size_t ch = 0; ch < s_max_channels; ++ch ) {

    const auto & channel_conf = board_conf->get_channel(ch);
    // get_channel returns the running value if the bool argument is true,
    // and the default values when the bool argument is false
    // hence, this loop does both the job of enabling and disebling
    
    result = m_interface->send_command(fmt::format( "WR TRIM CH {} V {}",
						    ch,
						    channel_conf.get_trim() ) );
    TLOG() << result.command << " -> " << result.result;
    
    result = m_interface->send_command(fmt::format("WR OFFSET CH {} V {}",
						   ch,
						   channel_conf.get_offset() ) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command(fmt::format("CFG OFFSET CH {} GAIN {}",
						     ch,
						     channel_conf.get_gain() ) );
    TLOG() << result.command << " -> " << result.result;
    
  } // channel loop

  // to check if the configuration went throguh we can
  //cmd (thing, "RD OFFSET CH " + std::to_string(ch), true);
  // But Manuel said that this is not necessary to be done all the time

  for ( size_t afe = 0; afe < s_max_afes ; ++afe) {

    const auto & afe_conf = board_conf->get_afe(afe);

    result = m_interface -> send_command( fmt::format("WR AFE {} REG 52 V {}",
						      afe,
						      afe_conf.get_lna()->get_reg52()) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command( fmt::format("WR AFE {} REG 4 V {}",
						      afe,
						      afe_conf.get_adc()->get_reg4()) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command( fmt::format("WR AFE {} REG 51 V {}",
						      afe,
						      afe_conf.get_pga()->get_reg51()) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command( fmt::format("WR AFE {} VGAIN V {}",
						      afe,
						      afe_conf.get_attenuator() ) );
    TLOG() << result.command << " -> " << result.result;

    result = m_interface -> send_command( fmt::format("WR BIASSET AFE {} V {}",
						      afe,
						      afe_conf.get_v_bias() ) );
    TLOG() << result.command << " -> " << result.result;

  } // afe loop
  
  //   // To check these values we can do things like
  //   // cmd (thing, "RD AFE " + std::to_string(AFE) + " REG 52", true);
  //   // for all these registers and get the values from the replies

  TLOG() << get_name() << ": done donfiguring analog chain";

  
}


void DaphneV2ControllerModule::align_DDR() {

  TLOG() << get_name() << ": aligning DDR";
  
  m_interface->write_register(0x2001, {1234});
  m_interface->write_register(0x2001, {1234});
  m_interface->write_register(0x2001, {1234});
  // this is correct to be done 3 times

  // wriring in regiester 0x2001 for 3 times resets every counter so we reset the counters on the Module side as well
  // to aling with the board
  for ( auto & c : m_channel_counters ) {
    c.triggers = 0;
    c.packets = 0;
  }
  m_last_package_counter = 0;
  
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  // this is necessary to give time to the board to align the AFE DDR
  // Otherwise further checks become pointless

  // --------------------------------------------
  // checking if the alignement is achieved
  // --------------------------------------------
  m_interface->write_register(0x2000, {1234});
  // this trigger the spy buffers
    
  // read register ch 8 of each afe,   by looping on all the afe we use
  auto board_conf = m_module_config->get_board_conf();
  for ( size_t afe = 0; afe < s_max_afes ; ++afe ) {
    if ( board_conf -> is_afe_used(afe) ) {
      auto data = m_interface->read_register(0x40000000 + (afe * 0x100000) + (8 * 0x10000), 15);  // ch = 8

      // things are ok when the data is 0x3f80
      if ( data[0] != DaphneV2ControllerModule::s_frame_alignment_good ) 
	throw DDRNotAligned(ERS_HERE, m_module_config->get_slot(), afe, data[0] );
    } //afe used
  }

  TLOG() << get_name() << ": done aligning DDR";
}


void
DaphneV2ControllerModule::configure_trigger_mode() {

  TLOG() << get_name() << ": Setting trigger mode";

  auto c = m_module_config->get_board_conf();

  auto threshold = c->get_self_trigger_threshold();
  
  if ( threshold > 0 ) {
    // se are in self trigger mode
    m_interface->write_register(0x3001, {0x3});  // only link0 is enabled
    m_interface->write_register(0x6000, {threshold});

    std::bitset<DaphneV2ControllerModule::s_max_channels> mask;
    auto board_conf = m_module_config->get_board_conf();
    // we unmask all the channels that are enabled
    for ( ChannelId ch = 0; ch < s_max_channels; ++ch ) {
      if ( board_conf->is_channel_used(ch) )
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
    auto full_stream_channels = c->get_full_stream_channels();
    for ( const auto & ch : full_stream_channels ) {

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
    } // loop over full_stream channels
  }

  TLOG() << get_name() << ": trigger mode configured";
  
}


// void
// DaphneV2ControllerModule::dump_buffers(const data_t& conf_as_json)
// {
//   auto start_time = std::chrono::high_resolution_clock::now();
  
//   auto conf_as_cpp = conf_as_json.get<DaphneV2ControllerModule::DumpBuffers>();

//   // during dumping no other operations are allowed
//   const std::lock_guard<std::mutex> lock(m_mutex);

//   std::string file_name(conf_as_cpp.directory);
//   if ( file_name.back() != '/' ) file_name += '/';
//   file_name += "spy_buffers_" + std::to_string(m_module_config->get_slot());

//   auto t = std::time(nullptr);
//   auto tm = *std::localtime(&t);
//   std::ostringstream oss;
//   oss << std::put_time(&tm, "%Y-%m-%dT%H-%M-%S");
//   file_name += '_' + oss.str() + ".txt";

//   size_t entries = std::min(conf_as_cpp.n_samples, (decltype(conf_as_cpp.n_samples)) 1024);
//   const size_t max_batch_size = 50;
  
//   m_interface->write_register(0x2000, {1234});
//   // this triggers the spy buffers

//   std::ofstream file(file_name);
    
//   for ( size_t ch = 0; ch < m_channel_confs.size(); ++ch) {
//     if ( ! channel_used(ch) ) continue;
    
//     auto afe   = ch / 8;
//     auto ch_id = ch % 8;

//     file << "AFE "<< afe << " CH " << ch_id;
   
//     size_t counter = 0;
//     while ( counter < entries ) {
//       auto n_points = counter + max_batch_size > entries ? entries-counter : max_batch_size;
//       auto data = m_interface->read_register(0x40000000 + (afe * 0x100000) + (ch_id * 0x10000) + counter, n_points);
//       counter += n_points;
//       for ( const auto & e : data ) {
// 	file << " " << e;
//       }
     
//     } // loop over the queries for the same channel

//     file << std::endl;
//   } // loop over the channels 
  
//   auto end_time = std::chrono::high_resolution_clock::now();

//   auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
//   TLOG() << get_name() << ": buffers dumped in " << duration.count() << " microseconds";
  
// }

  
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneV2ControllerModule)
