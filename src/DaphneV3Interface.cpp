/** 
 * @file DaphneV3Interface.cpp
 *  
 * Implementations of DaphneV3Interface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 * received with this code.
 *
 */

#include "DaphneV3Interface.hpp"

#include "logging/Logging.hpp"
#include <fmt/format.h>

#include <regex>
#include <string>
#include <utility>

using namespace dunedaq::daphnemodules;
using namespace daphne;

DaphneV3Interface::DaphneV3Interface( std::string address,
				      std::string routing, 
				      std::chrono::milliseconds timeout)
  : m_context(1)
  , m_socket(m_context, zmq::socket_type::dealer)
  , m_timeout(timeout) {


  m_socket.set(zmq::sockopt::routing_id, routing);
  auto value = (int) m_timeout.count();  // NOLINT
  TLOG() << routing << " timeout set to " << value << " ms";
  m_socket.set(zmq::sockopt::rcvtimeo, value);
  m_socket.set(zmq::sockopt::sndtimeo, value);
  m_socket.set(zmq::sockopt::immediate, 1); // Don't queue messages to incomplete connections
  
  // find out if the address has a port with a regex
  static const std::regex ip_with_port(R"(^([^\/\s:]+)(?::(\d{1,5}))?$)");
  std::smatch string_values; 
  if (! std::regex_match( address, string_values, ip_with_port ) ) {
    throw InvalidIPAddress(ERS_HERE, address);
  }

  m_connection = string_values[2].matched ?
    fmt::format("tcp://{}", address) :
    fmt::format("tcp://{}:{}", address, s_default_control_port) ;
  TLOG() << "Connecting to: " << m_connection;
  
  m_socket.connect(m_connection);

  auto add = string_values[1];
  auto port = string_values[2].matched ? std::stoi(string_values[2]) : s_default_control_port;
      
  try {
    if ( ! validate_connection() ) {
      auto add = string_values[1];
      auto port = string_values[2].matched ? std::stoi(string_values[2]) : s_default_control_port;
      throw FailedPing(ERS_HERE, add, port );
    }
  } catch ( const ers::Issue & e ) {
    throw FailedPing(ERS_HERE, add, port, e );
  }
    
}


void DaphneV3Interface::close() {

  const std::lock_guard<std::mutex> lock(m_access_mutex);
  m_socket.set(zmq::sockopt::linger, 0);
  m_socket.close();
  
}


ControlEnvelopeV2 DaphneV3Interface::send( std::string && message, daphne::MessageTypeV2 type) {

  const std::lock_guard<std::mutex> lock(m_access_mutex);
  
  _send(std::move(message), type);

  return _receive();
}


void DaphneV3Interface::_send( std::string && message, daphne::MessageTypeV2 type) {

  ControlEnvelopeV2 env;
  env.set_version(2);
  env.set_dir(DIR_REQUEST);

  env.set_type(type);
  env.set_payload(message);

  // additional information
  env.set_msg_id( m_message_counter++ );
  env.set_timestamp_ns(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());

  
  std::string bytes = env.SerializeAsString();

  if (! m_socket.send(zmq::buffer(bytes),  zmq::send_flags::none)) {
    throw FailedSend(ERS_HERE, MessageTypeV2_Name(type) );
  }
}

ControlEnvelopeV2 DaphneV3Interface::_receive() {

  zmq::message_t reply;
  if (! m_socket.recv(reply, zmq::recv_flags::none)) {
    // timeout or EAGAIN
    throw FailedReceive(ERS_HERE, m_connection);
  }
  if (reply.size() <= 0) {
    throw EmptyPayload(ERS_HERE, m_connection);
  }
  
  ControlEnvelopeV2 rep;
  if (!rep.ParseFromArray(reply.data(), static_cast<int>(reply.size()) )) {
    throw FailedDecoding(ERS_HERE, rep.GetTypeName(), reply.to_string());
  }
  
  if (rep.version() != 2 || rep.dir() != DIR_RESPONSE) {
    ers::warning(UnexpectedDirection(ERS_HERE, rep.version(), Direction_Name(rep.dir()), reply.to_string() )); 
  }

  return rep;
}


bool DaphneV3Interface::validate_connection()
{
  static const uint64_t good_value = 0xdeadbeef;  // NOLINT

  TestRegRequest req; // empty
  auto reply = send<TestRegResponse>( req.SerializeAsString(),
				      MessageTypeV2::MT2_READ_TEST_REG_REQ,
				      MessageTypeV2::MT2_READ_TEST_REG_RESP );
  
  return reply.value() == good_value;
}



