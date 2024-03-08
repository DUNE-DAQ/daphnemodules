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

#include "DaphneInterface.hpp"

#include <string>
#include <logging/Logging.hpp>

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
  daphnecontrollerinfo::Info info;
  info.total_amount = m_total_amount;
  info.amount_since_last_get_info_call = m_amount_since_last_get_info_call.exchange(0);

  ci.add(info);
}

void
DaphneController::do_conf(const data_t& conf_as_json)
{
  auto conf_as_cpp = conf_as_json.get<daphnecontroller::Conf>();

  auto ip = conf_as_cpp.daphne_address;
  TLOG() << "Using daphne at " << ip; 

  DaphneInterface di( ip.c_str(), 2001);

  auto res = di.read_register(0x9000, 1);
  for ( auto v : res ) {
    TLOG() << v;
  }
  
  res = di.read_buffer(0x40000000,15);
  for ( auto v : res ) {
    TLOG() << v;
  }


  auto cmd_res = di.send_command("RD VM ALL");
  TLOG() << cmd_res.command ;
  TLOG() << cmd_res.result ;
  
}
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
