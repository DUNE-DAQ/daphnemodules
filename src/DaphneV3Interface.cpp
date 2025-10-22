/** 
 *  
 * Implementations of DaphneV3Interface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

#include "DaphneV3Interface.hpp"
#include "logging/Logging.hpp"

#include <fmt/format.h>
#include <regex>

#warning CHECK IF WE NEED THIS
#include <sys/time.h>

using namespace dunedaq::daphnemodules;

DaphneV3Interface::DaphneV3Interface( std::string address,
				      std::string routing, 
				      std::chrono::milliseconds timeout)
  : m_context(1)
  , m_socket(m_context, zmq::socket_type::dealer)
  , m_timeout(timeout) {


  m_socket.set(zmq::sockopt::routing_id, routing);
  auto value = (int) timeout.count();
  m_socket.set(zmq::sockopt::rcvtimeo, value);
  m_socket.set(zmq::sockopt::sndtimeo, value);
  
  
  // find out if the address has a port with a regex
  static const std::regex ip_with_port("^([^/\s:]+)(?::(\d{1,5}))?$");
  std::smatch string_values; 
  if (! std::regex_match( address, string_values, ip_with_port ) ) {
    throw InvalidAddress(ERS_HERE, address);
  }

  auto connection = string_values.size() > 2 ?
    fmt::format("tcp://{}", address) :
    fmt::format("tcp://{}:{}", address, s_default_control_port) ;
  
  m_socket.connect(connection);

  if ( ! validate_connection() ) {
    auto add = string_values[1];
    auto port = string_values.size() > 2 ? std::stoi(string_values[2]) : s_default_control_port;
    throw FailedPing(ERS_HERE, add, port );
  }

}


void DaphneV3Interface::close() {

  const std::lock_guard<std::mutex> lock(m_access_mutex);
  m_socket.set(zmq::sockopt::linger, 0);
  m_socket.close();
  
}


bool DaphneV3Interface::validate_connection() const {

  static const uint64_t good_value = 0xdeadbeef; 

  #warning FIX ME
  return true;
}







