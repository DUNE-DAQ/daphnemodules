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

  

} // dunedaq namespace

#endif // COMMONISSUES_HPP
