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
#include "oei.hpp"
#include "cmd.hpp"
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
  auto ips = conf_as_cpp.daphne_list;
  for (const auto&ip:ips) {
	OEI thing(ip.c_str());
        TLOG() << "Configuring Offset in 40 ch DAPHNE " << ip ;

         std::string initial_command = "CFG AFE ALL INITIAL";
         cmd (thing, initial_command);

         int nChannels = 40;
         for (int ch = 0; ch < nChannels; ++ch) {
             cmd (thing, "WR OFFSET CH " + std::to_string(ch) + " V 1468", true);
             cmd (thing, "CFG OFFSET CH " + std::to_string(ch) + " GAIN 2", true);
         }
         TLOG() << "Configuring AFE registers 4, 51, 52 and Attenuators";

         int nAFEs = 5;
         for (int AFE = 0; AFE < nAFEs; ++AFE) {
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 52 V 21056", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 4 V 24", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 51 V 16", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " VGAIN V 2667", true);

	}
        thing.closes();
}
}
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneController)
