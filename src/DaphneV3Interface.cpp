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
using namespace daphne;

DaphneV3Interface::DaphneV3Interface( std::string address,
				      std::string routing, 
				      std::chrono::milliseconds timeout)
  : m_context(1)
  , m_socket(m_context, zmq::socket_type::dealer)
  , m_timeout(timeout) {


  const std::lock_guard<std::mutex> lock(m_access_mutex);
  
  m_socket.set(zmq::sockopt::routing_id, routing);
  auto value = (int) timeout.count();
  TLOG() << routing << " timeout set to " << value << " ms";
  m_socket.set(zmq::sockopt::rcvtimeo, value);
  m_socket.set(zmq::sockopt::sndtimeo, value);
  
  
  // find out if the address has a port with a regex
  static const std::regex ip_with_port(R"(^([^\/\s:]+)(?::(\d{1,5}))?$)");
  std::smatch string_values; 
  if (! std::regex_match( address, string_values, ip_with_port ) ) {
    throw InvalidIPAddress(ERS_HERE, address);
  }

  TLOG() << "Argument size: " << string_values.size();
  m_connection = string_values[2].matched ?
    fmt::format("tcp://{}", address) :
    fmt::format("tcp://{}:{}", address, s_default_control_port) ;
  TLOG() << "Connecting to : " << m_connection;
  
  m_socket.connect(m_connection);

  if ( ! validate_connection() ) {
    auto add = string_values[1];
    auto port = string_values[2].matched ? std::stoi(string_values[2]) : s_default_control_port;
    throw FailedPing(ERS_HERE, add, port );
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
  #warning ADD TIME and possibly other missing types
  
  std::string bytes = env.SerializeAsString();

  if (! m_socket.send(zmq::buffer(bytes), zmq::send_flags::none)) {
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
  if (!rep.ParseFromArray(reply.data(), (int)reply.size() )) {
    throw FailedDecoding(ERS_HERE, rep.GetTypeName(), reply.to_string());
  }
  
  if (rep.version() != 2 || rep.dir() != DIR_RESPONSE) {
    ers::warning(UnexpectedDirection(ERS_HERE, rep.version(), Direction_Name(rep.dir()), reply.to_string() )); 
  }

  return rep;
}

template<class T>
T DaphneV3Interface::send( std::string && message, daphne::MessageTypeV2 sent_type, daphne::MessageTypeV2 received_type ) {

  std::unique_lock<std::mutex> lock(m_access_mutex);

  _send(std::move(message), sent_type);

  auto ret = _receive();

  lock.unlock();

  const auto ty = ret.type();
  T out;
  if ( ty != received_type ) {
    throw FailedDecoding(ERS_HERE, out.GetTypeName(), ret.payload(),
			 TypeMismatch(ERS_HERE, MessageTypeV2_Name(ty), MessageTypeV2_Name(received_type)) );
  }

  if (!out.ParseFromString(ret.payload())) {
    throw FailedDecoding(ERS_HERE, out.GetTypeName(), ret.payload());
  }

  return out;
}



bool DaphneV3Interface::read_test_register(uint64_t& value) const
{
  using namespace daphne; // protobuf package

  auto* sock = const_cast<zmq::socket_t*>(&m_socket);

  try {
    TestRegRequest req; // empty
    ControlEnvelopeV2 env;
    env.set_version(2);
    env.set_dir(DIR_REQUEST);

    env.set_type(static_cast<MessageTypeV2>(304));
    env.set_payload(req.SerializeAsString());


    std::string bytes = env.SerializeAsString();

    if (!sock->send(zmq::buffer(bytes), zmq::send_flags::none)) {
      return false;
    }

    zmq::message_t reply;
    if (!sock->recv(reply, zmq::recv_flags::none)) {
      return false; // timeout or EAGAIN
    }
    if (reply.size() <= 0) {
      return false;
    }

    ControlEnvelopeV2 rep;
    if (!rep.ParseFromArray(reply.data(), static_cast<int>(reply.size()))) {
      return false;
    }
    if (rep.version() != 2 || rep.dir() != DIR_RESPONSE) {
      return false;
    }

    const auto ty = rep.type();
    if (!(ty == MT2_READ_TEST_REG_RESP || ty == static_cast<MessageTypeV2>(305))) {
      return false;
    }

    TestRegResponse out;
    if (!out.ParseFromString(rep.payload())) {
      return false;
    }

    value = out.value();
    return true;

  } catch (const zmq::error_t&) {
    return false;
  } catch (...) {
    return false;
  }
}

bool DaphneV3Interface::validate_connection()
{
  static const uint64_t good_value = 0xdeadbeef;

  TestRegRequest req; // empty
  auto reply = send<TestRegResponse>( req.SerializeAsString(),
				      MessageTypeV2::MT2_READ_TEST_REG_REQ,
				      MessageTypeV2::MT2_READ_TEST_REG_RESP );
  
  return reply.value() == good_value;
}



