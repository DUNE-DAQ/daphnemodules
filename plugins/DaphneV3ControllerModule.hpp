#pragma once

#include "appfwk/DAQModule.hpp"
#include "MezzCommandBuilder.hpp"
#include "DaphneV2Interface.hpp" 

namespace dunedaq::daphnemodules {

class DaphneV3ControllerModule : public appfwk::DAQModule
{
public:
  explicit DaphneV3ControllerModule(const std::string& name);
  void init(std::shared_ptr<appfwk::ConfigurationManager>) override;

private:
  void do_conf(const CommandData_t&);
  void do_start(const CommandData_t&);
  void do_scrap(const CommandData_t&);

  std::unique_ptr<DaphneV2Interface> m_iface;
};

} 
