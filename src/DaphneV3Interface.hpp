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

#include "daphnemodules/CommonIssues.hpp"

namespace dunedaq {

  ERS_DECLARE_ISSUE( daphnemodules,
		     SocketCreationError,
		     "Failed to create a socket",
		     ERS_EMPTY
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedPing,
		     "Failed to ping daphne board at " << ip << ':' << port,
		     ((std::string)ip)((int)port)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedSend,
		     "Failed to send message of type " << type,
		     ((std::string)type)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedReceive,
		     "Failed to receive message from " << connection,
		     ((std::string)connection)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     EmptyPayload,
		     "Empty payload received from " << connection,
		     ((std::string)connection)
		     ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     TypeMismatch,
		     "Received message of type " << type << " instead of " << expected,
		     ((std::string)type)((std::string)expected)
		     ) 

  
  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedDecoding,
		     "Failed to de-serialise to " << type << ". Message: " << message,
		     ((std::string)type)((std::string)message)
		     ) 
  
  ERS_DECLARE_ISSUE( daphnemodules,
		     UnexpectedDirection,
		     "Message received with unexpeted properties. Version: " << version << ", direction " << ". Message: " << message,
		     ((uint32_t)version)((std::string)direction)((std::string)message)
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
    // It returns the serialised reply
    
    daphne::ControlEnvelopeV2 send( std::string && message, daphne::MessageTypeV2 );

    // this takes the serilised message and encodes it into the envelope
    // It returns the de-serialised objects
    template<class T>
    T send( std::string && message, daphne::MessageTypeV2 sent_type, daphne::MessageTypeV2 received_type );
    
    bool validate_connection();
    
  protected:
    void _send( std::string && message, daphne::MessageTypeV2 );
    daphne::ControlEnvelopeV2 _receive();
    
    void close();
    
  private:
    zmq::context_t m_context;
    
    zmq::socket_t m_socket;
    mutable std::mutex m_access_mutex;

    std::string m_connection;
    
    std::chrono::milliseconds m_timeout{1000};

    static const size_t s_default_control_port = 40001;
  }; 
  

} // namespce  dunedaq::daphnemodules

#include <DaphneV3Interface.hxx>

#endif // DAPHNEMODULES_SRC_DAPHNEV3INTERFACE_HPP_
