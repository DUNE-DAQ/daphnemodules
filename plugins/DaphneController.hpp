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
#include <mutex>

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
		     InvalidChannelConfiguration,
                     "Channel " << id << " has invalid configuration, trim: " << trim << ", offset: " << offset << ", gain:" << gain,
		     ((uint32_t)id)((uint32_t)trim)((uint32_t)offset)((uint32_t)gain)
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
		       TooManyChannels,
                     "Too many full stream channels. Total requested:  " << tot,
		     ((size_t)tot)
		     )

  
   ERS_DECLARE_ISSUE( daphnemodules,
                     InvalidBiasCtrlConfiguration,
                     "Invalid BiasCtrl Configuration " << v_biasctrl,
                     ((std::string)v_biasctrl)
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

  void init(const data_t&) override {;}

  void get_info(opmonlib::InfoCollector&, int /*level*/) override;

  DaphneController(const DaphneController&) = delete;
  DaphneController& operator=(const DaphneController&) = delete;
  DaphneController(DaphneController&&) = delete;
  DaphneController& operator=(DaphneController&&) = delete;

  ~DaphneController() = default;

private:

  using ChannelId = uint8_t;
  
  // Commands DaphneController can receive
  void do_conf(const data_t&);

  // specific actions
  void create_interface( const std::string & ip ) ;
  void validate_configuration(const daphnecontroller::Conf &);   
  void configure_timing_endpoints();
  void configure_analog_chain();
  void align_DDR();

  bool channel_used( ChannelId id ) const {
    return m_channel_confs[id].offset > 0;
  }
  
  std::unique_ptr<DaphneInterface> m_interface;
  std::mutex m_mutex;  // mutex for interface

  uint8_t  m_slot;
  uint16_t m_bias_ctrl;
  uint16_t m_self_threshold;
  
  static const ChannelId s_max_channels = 40;
  std::array<daphnecontroller::ChannelConf, s_max_channels> m_channel_confs;
  // this array is indexed in the [0-40) range

  struct AFEConf {
    uint16_t v_gain = 0;  // 12 bit register
    uint16_t v_bias = 0;  // 12 bit register
    uint8_t  reg4   = 0;  // 4  bit register
    uint16_t reg51 = 0;   // 14 bit register
    uint16_t reg52 = 0;   // 16 bit register
  };
  
  static const ChannelId s_max_afes = 5;
  std::array<AFEConf, s_max_afes> m_afe_confs;
  // mapping from the channels to the AFE
  // 0-7 -> AFE 0,  8-15 -> AFE 1, 16-23 -> AFE 2, 24-31 -> AFE 3, 32-39 -> AFE 4 

  std::vector<ChannelId> m_full_stream_channels;
  
  static const uint16_t s_frame_alignment_good = 0x3f80;

  // we need to have a mapping to the links from the configuration
  // 0x500X X in 0 to F

  

  
  
  
};

} // namespace dunedaq::daphnemodules

#endif // DAPHNEMODULES_PLUGINS_DAPHNECONTROLLER_HPP_
