#ifndef COMMONISSUES_HPP
#define COMMONISSUES_HPP

#include "ers/Issue.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

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

} // dunedaq namespace

#endif // COMMONISSUES_HPP
