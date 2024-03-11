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

  static std::regex volt_regex(".* VBIAS0= ([^ ]+) VBIAS1= ([^ ]+) VBIAS2= ([^ ]+) VBIAS3= ([^ ]+) VBIAS4= ([^ ]+) POWER.-5v.= ([^ ]+) POWER..2.5v.= ([^ ]+) POWER..CE.= ([^ ]+) TEMP.Celsius.= ([^ ]+) .*");

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
}

void
DaphneController::do_conf(const data_t& conf_as_json)
{
  auto conf_as_cpp = conf_as_json.get<daphnecontroller::Conf>();

  auto ip = conf_as_cpp.daphne_address;
  TLOG() << "Using daphne at " << ip; 

  m_interface.reset( new  DaphneInterface( ip.c_str(), 2001) );

  auto res = m_interface->read_register(0x9000, 1);
  for ( auto v : res ) {
    TLOG() << v;
  }
  
  res = m_interface->read_buffer(0x40000000,15);
  for ( auto v : res ) {
    TLOG() << v;
  }


  auto cmd_res = m_interface->send_command("RD VM ALL");
  TLOG() << cmd_res.command ;
  TLOG() << cmd_res.result ;
  
}
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
