/**
 * @file DaphneController.hpp
 *
 * Developer(s) of this DAQModule have yet to replace this line with a brief description of the DAQModule.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef DAPHNEMODULES_PLUGINS_DAPHNECONTROLLER_HPP_
#define DAPHNEMODULES_PLUGINS_DAPHNECONTROLLER_HPP_

#include "appfwk/DAQModule.hpp"

#include <atomic>
#include <limits>
#include <string>
#include <array>

#include "DaphneInterface.hpp"

#include "daphnemodules/daphnecontroller/Structs.hpp"

namespace dunedaq {
  ERS_DECLARE_ISSUE( daphnemodules,
                     WrongMonitoringString,
                     "Response from board was not parsed correctly",
                     ((std::string)string)
                   )

  ERS_DECLARE_ISSUE( daphnemodules,
                     FailedStringConversion,
                     "String " << str << " failed to be converted into a number",
		     ((std::string)str)
                   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidSlot,
                     "Invalid slot " << slot << " obtained from IP " << ip,
		     ((uint8_t)slot) ((std::string)ip)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     PLLNotLocked,
                     mm << " not locked",
		     ((std::string)mm)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     TimingEndpointNotReady,
                     "Timing endpoint not ready, full status: " << status,
		     ((std::string)status)
		   )
  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidChannelConfiguration,
                     "Channel " << id << "has invalid configuration, offset: " << offset << ", gain:" << gain,
		     ((uint32_t)id)((uint32_t)offset)((uint32_t)gain)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidAFEConfiguration,
                     "AFE " << id << "has invalid configuration, reg52: " << reg52
		            << ", reg4: "  << reg4
                            << ", reg51: " << reg51
		     << ", vgain: " << vgain,
		     ((uint32_t)id)((uint32_t)reg52)((uint32_t)reg4)((uint32_t)reg51)((uint32_t)vgain)
		   )

  ERS_DECLARE_ISSUE( daphnemodules,
		     DDRNotAligned,
                     "AFE "<< afe << " DDR not aligned, check value: " << check,
		     ((uint16_t)afe)((uint64_t)check)
		   )
  
}

namespace dunedaq::daphnemodules {

class DaphneController : public dunedaq::appfwk::DAQModule
{
public:
  explicit DaphneController(const std::string& name);

  void init(const data_t&) override;

  void get_info(opmonlib::InfoCollector&, int /*level*/) override;

  DaphneController(const DaphneController&) = delete;
  DaphneController& operator=(const DaphneController&) = delete;
  DaphneController(DaphneController&&) = delete;
  DaphneController& operator=(DaphneController&&) = delete;

  ~DaphneController() = default;

private:

  // Commands DaphneController can receive
  void do_conf(const data_t&);

  // specific actions
  void configure_timing_endpoints();
  void configure_analog_chain();
  void align_DDR();
  
  std::unique_ptr<DaphneInterface> m_interface;
  uint8_t m_slot;

  static const int s_max_channels = 40;
  std::array<daphnecontroller::ChannelConf, s_max_channels> m_channel_confs;
  // this array is indexed in the [0-40) range

  static const int s_max_afes = 5;
  std::array<daphnecontroller::AFEConf, s_max_afes> m_afe_confs;
  // mapping from the channels to the AFE
  // 0-7 -> AFE 0,  8-15 -> AFE 1, 16-23 -> AFE 2, 24-31 -> AFE 3, 32-39 -> AFE 4 
  
  static const uint16_t s_frame_alignment_good = 0x3f80;

  // we need to have a mapping to the links from the configuration
  // 0x500X X in 0 to F

  

  
  
  
};

} // namespace dunedaq::daphnemodules

#endif // DAPHNEMODULES_PLUGINS_DAPHNECONTROLLER_HPP_
