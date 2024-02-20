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

namespace dunedaq::daphnemodules {

DaphneController::DaphneController(const std::string& name)
  : dunedaq::appfwk::DAQModule(name)
{
  register_command("conf", &DaphneController::do_conf);
}

void
DaphneController::init(const data_t& /* structured args */)
{
  TLOG() << "Test";
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
  auto ip = conf_as_cpp.daphne_ip;
  //m_some_configured_value = conf_as_cpp.some_configured_value;
}

} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
