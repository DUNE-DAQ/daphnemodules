/** 
 *  
 * Implementations of DaphneV3Interface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

#include "DaphneV3Interface.hpp"
#include "logging/Logging.hpp"
#include "daphnemodules/daphne_control_high.pb.h"
#include <fmt/format.h>
#include <regex>

#warning CHECK IF WE NEED THIS
#include <sys/time.h>

using namespace dunedaq::daphnemodules;

DaphneV3Interface::DaphneV3Interface( std::string address,
				      std::chrono::milliseconds timeout)
  : m_context(1)
  , m_socket(m_context, zmq::socket_type::dealer)
  , m_timeout(timeout) {

  
  // Normalize to tcp://host(:port)
  const bool has_scheme = address.rfind("tcp://", 0) == 0;
  std::string addr = has_scheme ? address.substr(6) : address;
  const bool has_port = (addr.find(':') != std::string::npos);
  auto connection = has_port ? fmt::format("tcp://{}", addr)
                             : fmt::format("tcp://{}:{}", addr, s_default_control_port);
  m_socket.set(zmq::sockopt::rcvtimeo, static_cast<int>(m_timeout.count()));
  m_socket.set(zmq::sockopt::sndtimeo, static_cast<int>(m_timeout.count()));
  m_socket.set(zmq::sockopt::routing_id, "daphne-v3-iface");
  m_socket.connect(connection);
  // optional health check
  // if (!validate_connection()) throw FailedPing(ERS_HERE, addr, has_port ? std::stoi(addr.substr(addr.find(':')+1)) : s_default_control_port);

  // if ( ! validate_connection() )
    //throw FailedPing(ERS_HERE, ipaddr, port );

}


void DaphneV3Interface::close() {

  const std::lock_guard<std::mutex> lock(m_access_mutex);
  m_socket.set(zmq::sockopt::linger, 0);
  m_socket.close();
  
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

bool DaphneV3Interface::validate_connection() const
{
  static const uint64_t good_value = 0xdeadbeef;
  uint64_t val = 0;
  if (!read_test_register(val)) return false;
  return val == good_value;
}






