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

#include "DaphneInterface.hpp"

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

  std::unique_ptr<DaphneInterface> m_interface;
  uint8_t m_slot;
  static int s_max_channels = 40;
  // somehow we  need to store which channles are used

  // all the values have to come from configuration
  // CH OFFSET 2700 if GAIN is 1, 1500 if GAIN is 2

  static uint16_t s_frame_alignment_error = 0x3f80;

  
  
  
};

} // namespace dunedaq::daphnemodules

#endif // DAPHNEMODULES_PLUGINS_DAPHNECONTROLLER_HPP_
