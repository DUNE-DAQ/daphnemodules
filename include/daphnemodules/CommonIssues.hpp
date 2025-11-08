/**
 * @file CommonIssues.hpp
 *
 * Developer(s) of this DAQModule have yet to replace this line with a brief description of the DAQModule.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef DAPHNEMODULES_INCLUDE_DAPHNEMODULES_COMMONISSUES_HPP_ 
#define DAPHNEMODULES_INCLUDE_DAPHNEMODULES_COMMONISSUES_HPP_ 

#include "ers/Issue.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

#include <string>

namespace dunedaq {

  ERS_DECLARE_ISSUE( daphnemodules,
                     ConfigurationFailed,
                     name << " failed to retrieve its conf object",
                     ((std::string)name)
                   )

  ERS_DECLARE_ISSUE( daphnemodules,
                     InvalidIPAddress,
                     "Invalid address: " << ip,
                     ((std::string)ip)
                   ) 
  
    ERS_DECLARE_ISSUE( daphnemodules,
		       InvalidChannelConfiguration,
		       "Channel " << id << " has invalid configuration, trim: " << trim << ", offset: " << offset << ", gain:"<< gain,
		       ((uint32_t)id)((uint32_t)trim)((uint32_t)offset)((uint32_t)gain)
		       )

  
  ERS_DECLARE_ISSUE( daphnemodules,
		     TooManyChannels,
                     "Too many full stream channels. Total requested:  " << tot,
                     ((size_t)tot)
                     )

  ERS_DECLARE_ISSUE( daphnemodules,
                     UnsuccessfulConfiguration,
                     name << ": board reports configuration failure. Message: " << message,
                     ((std::string)name) ((std::string)message)
		     )


  // Monitoring 
  ERS_DECLARE_ISSUE( daphnemodules,
                     MonitoringFailed,
                     "Monitoring of " << item << " failed",
                     ((std::string)item)
                   )
  
  ERS_DECLARE_ISSUE_BASE( daphnemodules,
			  TriggerMonitoringFailed,
			  MonitoringFailed,
			  item << " failed to retrieve reponse for trigger snapshots. Message: " << message,
			  ((std::string)item),
			  ((std::string)message)
                   )


} // namespace dunedaq

#endif // DAPHNEMODULES_INCLUDE_DAPHNEMODULES_COMMONISSUES_HPP_
