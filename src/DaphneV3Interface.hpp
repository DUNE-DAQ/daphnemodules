/**
 * @file DaphneV2Interface.hpp
 *
 * Definition of the interface protocol to the daphne V3 boards
 *  It's basically just a wrapper around a zmq socket
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 *
 */

#ifndef DAPHNEMODULES_SRC_DAPHNEV3INTERFACE_HPP_
#define DAPHNEMODULES_SRC_DAPHNEV3INTERFACE_HPP_ 

#include <memory>
#include <mutex>
#include <functional>


#include <ers/ers.hpp>
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

#include <zmq.hpp>

#include "daphnemodules/daphne_control_high.pb.h"



namespace dunedaq {

  ERS_DECLARE_ISSUE( daphnemodules,
		     SocketCreationError,
		     "Failed to create a socket",
		     ERS_EMPTY
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidAddress,
		     "Invalid address: " << address,
		     ((std::string)address)
		   ) 
  
  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedPing,
		     "Failed to ping daphne board at " << ip << ':' << port,
		     ((std::string)ip)((int)port)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedSocketInteraction,
		     "Failed to call " << command,
		     ((std::string)command)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     CommandTimeout,
		     "Command " << command << " timed out after " << timeout_us << " microseconds",
		     ((std::string)command)((unsigned int)timeout_us)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     SocketTimeout,
		     "Socket timed out after " << timeout_us << " microseconds",
		     ((unsigned int)timeout_us)
		   ) 
 
  } // dunedaq namespace


namespace dunedaq::daphnemodules {

  class DaphneV3Interface {

  public:
    DaphneV3Interface( std::string address,  // it can contain the port or not
		       std::string rounting,  // this should be the name of the controller module
		       std::chrono::milliseconds timeout = std::chrono::milliseconds(500));
    

    ~DaphneV3Interface() { close(); }

    DaphneV3Interface(const DaphneV3Interface &) = delete;
    DaphneV3Interface & operator= (const DaphneV3Interface & ) = delete;
    DaphneV3Interface(DaphneV3Interface &&) = delete;
    DaphneV3Interface & operator= (DaphneV3Interface &&) = delete;

    // this takes the serilised message and encodes it into the envelope
    std::string send( std::string && message, daphne::MessageTypeV2 );
    
    bool validate_connection() const ;

    bool read_test_register(uint64_t& value) const;
    
  protected:
    void _send( std::string && message, daphne::MessageTypeV2 );
    std::string _receive();
    
    void close();
    
  private:
    zmq::context_t m_context;
    
    zmq::socket_t m_socket; 
    mutable std::mutex m_access_mutex;
    
    std::chrono::milliseconds m_timeout{1000};

    static const size_t s_default_control_port = 40001;
  }; 
  

} // namespce  dunedaq::daphnemodules


#endif // DAPHNEMODULES_SRC_DAPHNEV3INTERFACE_HPP_
