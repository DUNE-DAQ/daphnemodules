/**
 * @file DaphneV2ControllerModule.hpp
 *
 * Developer(s) of this DAQModule have yet to replace this line with a brief description of the DAQModule.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef DAPHNEMODULES_PLUGINS_DAPHNEV2CONTROLLERMODULE_HPP_ 
#define DAPHNEMODULES_PLUGINS_DAPHNEV2CONTROLLERMODULE_HPP_


#include "DaphneV2ControllerModule.hpp"
#include "appfwk/DAQModule.hpp"

#include "DaphneV2Interface.hpp"
#include "daphnemodules/CommonIssues.hpp"

#include "daphnemodules/opmon/DaphneControllerModule.pb.h"

#include "appmodel/DaphneBoard.hpp"
#include "appmodel/DaphneV2BoardConf.hpp"
#include "appmodel/DaphneV2ControllerModule.hpp"

#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

#include <atomic>
#include <limits>
#include <string>
#include <array>
#include <mutex>
#include <memory>


namespace dunedaq {
  
  ERS_DECLARE_ISSUE( daphnemodules,
                     WrongMonitoringString,
                     "Board " << id
		     << ": response from board was not parsed correctly for "
		     << counter << " times. Last Rseponse: " << response,
                     ((std::string)id)((uint16_t)counter)((std::string)response)
                   )

  ERS_DECLARE_ISSUE( daphnemodules,
                     FailedStringConversion,
                     "String " << str << " failed to be converted into a number",
		     ((std::string)str)
                   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidSlot,
                     "Invalid slot " << slot << " obtained from IP " << ip,
		     ((uint16_t)slot) ((std::string)ip)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     PLLNotLocked,
                     "Board in slot " << slot << ": " << mm << " not locked",
		     ((uint16_t)slot)((std::string)mm)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     TimingEndpointNotReady,
                     "Board in slot " << slot << ": timing endpoint not ready, full status: " << status,
		     ((uint16_t)slot)((std::string)status)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidBiasControl,
                     bias << " bigger than 4095",
		     ((uint64_t)bias)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidChannelId,
                     "Channel " << id <<'/' << max << " not available", 
		     ((uint32_t)id)((uint32_t)max)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidAFEVoltage,
                     "AFE " << id << " has invalid voltage, gain: " << gain << ", bias: " << bias,
		     ((uint32_t)id)((uint32_t)gain)((uint32_t)bias)
		   )
  
  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidPGAConf,
                     "AFE " << id << " has invalid PGA conf (reg51), cut selection: " << cut,
		     ((uint32_t)id)((uint32_t)cut)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidLNAConf,
                     "AFE " << id << " has invalid LNA conf (reg52), clamp: " << clamp
		     << ", gain: " << gain,
		     ((uint32_t)id)((uint32_t)clamp)((uint32_t)gain)
		     )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidThreshold,
                     "Invalid threshold: " << threshold,
		     ((uint32_t)threshold)
		     )
  
   ERS_DECLARE_ISSUE( daphnemodules,
                     InvalidBiasCtrlConfiguration,
                     "Invalid BiasCtrl Configuration " << v_biasctrl,
                     ((std::string)v_biasctrl)
                   )
  
  ERS_DECLARE_ISSUE( daphnemodules,
		     DDRNotAligned,
                     "board in slot " << slot << ": AFE " << afe << " DDR not aligned, check value: " << check,
		     ((uint16_t)slot)((uint16_t)afe)((uint64_t)check)
		   )
  
}  // namespace dunedaq

namespace dunedaq::daphnemodules {

class DaphneV2ControllerModule : public dunedaq::appfwk::DAQModule
{
public:
  explicit DaphneV2ControllerModule(const std::string& name);

  void init(std::shared_ptr<appfwk::ConfigurationManager> cfgMgr) override;

  void generate_opmon_data() override;

  DaphneV2ControllerModule(const DaphneV2ControllerModule&) = delete;
  DaphneV2ControllerModule& operator=(const DaphneV2ControllerModule&) = delete;
  DaphneV2ControllerModule(DaphneV2ControllerModule&&) = delete;
  DaphneV2ControllerModule& operator=(DaphneV2ControllerModule&&) = delete;

  ~DaphneV2ControllerModule() = default;

private:

  using ChannelId = uint8_t;  // NOLINT
  
  // Commands DaphneV2ControllerModule can receive
  void do_conf(const CommandData_t&);
  void do_start(const CommandData_t&);
  void do_scrap(const CommandData_t&);
  //  void dump_buffers(const CommandData_t&);
  
  // specific actions
  void create_interface( const std::string & ip,
			 std::chrono::milliseconds timeout )  ;
  void validate_configuration(const appmodel::DaphneV2BoardConf &) const;   
  void configure_timing_endpoints();
  void configure_analog_chain(bool intial_config);
  void align_DDR();
  void configure_trigger_mode();
  void disable_links();
  void reset_counters();
  
  std::unique_ptr<DaphneV2Interface> m_interface;
  std::mutex m_mutex;  // mutex for interface
  std::atomic<bool> m_scrap_called = false;

  static const ChannelId s_max_channels = 40;
  static const ChannelId s_max_afes = 5;
  using conf_t = appmodel::DaphneV2ControllerModule;
  const conf_t* m_module_config = nullptr;
  
  static const uint16_t s_frame_alignment_good = 0x3f80;

  uint16_t m_error_counter = 0;  // NOLINT
  // counter use to see how many times we failed the parsing of the monitoing

  //monitoring
  using const_metric_counter_t = std::invoke_result<decltype(&dunedaq::daphnemodules::opmon::ChannelInfo::total_triggers),
						    dunedaq::daphnemodules::opmon::ChannelInfo>::type;
  using counter_t = std::remove_const<const_metric_counter_t>::type;
  struct Counters {
    std::atomic<counter_t> triggers = 0;
    std::atomic<counter_t> packets  = 0;
  };
  std::array<Counters, s_max_channels> m_channel_counters;
  std::atomic<counter_t> m_last_package_counter = 0;
  std::atomic<counter_t> m_last_unsent_counter = 0;
  
    const appmodel::DaphneBoard* m_board{nullptr};
    const appmodel::DaphneV2BoardConf* m_board_conf{nullptr};
};

} // namespace dunedaq::daphnemodules

#endif // DAPHNEMODULES_PLUGINS_DAPHNEV2CONTROLLERMODULE_HPP_ 
