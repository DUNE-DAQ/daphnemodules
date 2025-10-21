#pragma once

#include "appfwk/DAQModule.hpp"
#include "MezzCommandBuilder.hpp"
#include "DaphneV3Interface.hpp"

#include "daphnemodules/CommonIssues.hpp"
#include "daphnemodules/daphne_control_high.pb.h"
#include "appmodel/DaphneV3ControllerModule.hpp"

namespace dunedaq {

  namespace daphnemodules {

class DaphneV3ControllerModule : public appfwk::DAQModule
{
public:
  explicit DaphneV3ControllerModule(const std::string& name);
  void init(std::shared_ptr<appfwk::ConfigurationManager>) override;

private:
  void do_conf(const CommandData_t&);
  void do_start(const CommandData_t&);
  void do_scrap(const CommandData_t&);

  void configure_analog_chain(bool intial_config);
  
  std::unique_ptr<DaphneV3Interface> m_iface;

  using conf_t = appmodel::DaphneV3ControllerModule;
  const conf_t* m_module_config = nullptr;

  using const_channel_id_t = std::invoke_result<decltype(&daphne::ChannelConfig::id),
						daphne::ChannelConfig>::type;
  using ChannelId = std::remove_const<const_channel_id_t>::type;
  static const ChannelId s_max_channels = 40;

  using const_afe_id_t = std::invoke_result<decltype(&daphne::AFEConfig::id),
					    daphne::AFEConfig>::type;
  using AFEId = std::remove_const<const_afe_id_t>::type;

  static const AFEId s_max_afes = 5;

  static const size_t s_default_control_port = 40001;
  
};

  }  // daphnemodules namespace
} // dunedaq namespace
