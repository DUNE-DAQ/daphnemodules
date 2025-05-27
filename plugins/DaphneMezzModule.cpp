#include "DaphneMezzModule.hpp"
#include "logging/Logging.hpp"

namespace dunedaq::daphnemodules {

DaphneMezzModule::DaphneMezzModule(const std::string& name)
  : appfwk::DAQModule(name)
{
  register_command("conf",  &DaphneMezzModule::do_conf);
  register_command("start", &DaphneMezzModule::do_start);
  register_command("scrap", &DaphneMezzModule::do_scrap);
}

void DaphneMezzModule::init(std::shared_ptr<appfwk::ConfigurationManager> cfg)
{
}

void DaphneMezzModule::do_conf(const data_t& /*payload*/)
{
  // Hard-wired demo: set bias on AFE0 to 800 mV
  if (!m_iface)
    m_iface = std::make_unique<DaphneV2Interface>("127.0.0.1", 2001);

  auto cmd = mezz::CommandBuilder::build_bias_cmd(/*afe*/0, /*mv*/800);
  m_iface->send_command_retry(cmd, /*retry*/3);
}

void DaphneMezzModule::do_start(const data_t&)  { /* nothing yet */ }
void DaphneMezzModule::do_scrap(const data_t&)  { m_iface.reset(); }

} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneMezzModule)
