#include "daphnemodules/zmqclient/ControlClient.hpp"
#include <fmt/core.h>           // or ers::logging

namespace daphne::zmq {

// ----------------------------------------------------------------------
// helper – builds envelope and serialises
// ----------------------------------------------------------------------
namespace {
template <typename RequestT>
std::string
make_envelope_bytes(daphnemodules::MessageType type, const RequestT& req)
{
  daphnemodules::ControlEnvelope env;
  env.set_type(type);
  env.set_payload(req.SerializeAsString());

  std::string bytes;
  env.SerializeToString(&bytes);
  return bytes;                 // NRVO / move-elided
}

template <typename ResponseT>
ResponseT
parse_response(const zmq::message_t& msg, daphnemodules::MessageType expected)
{
  daphnemodules::ControlEnvelope env;
  env.ParseFromArray(msg.data(), static_cast<int>(msg.size()));

  if (env.type() != expected) {
    throw std::runtime_error(
      fmt::format("Unexpected envelope type {} (expected {})",
                  env.type(), expected));
  }
  ResponseT rsp;
  rsp.ParseFromString(env.payload());
  return rsp;
}
} // namespace
// ----------------------------------------------------------------------

ControlClient::ControlClient(zmq::context_t& ctx,
                             std::string_view ip,
                             uint16_t         port,
                             Milliseconds     timeout)
  : socket_(ctx, zmq::socket_type::req)
{
  socket_.set(zmq::sockopt::rcvtimeo, static_cast<int>(timeout.count()));
  socket_.set(zmq::sockopt::sndtimeo, static_cast<int>(timeout.count()));
  socket_.connect(fmt::format("tcp://{}:{}", ip, port));
}

// ---------------------- API -------------------------------------------

daphnemodules::ConfigureResponse
ControlClient::configure(const daphnemodules::ConfigureRequest& req)
{
  auto bytes = make_envelope_bytes(daphnemodules::CONFIGURE_FE, req);
  socket_.send(zmq::buffer(bytes), zmq::send_flags::none);

  zmq::message_t reply;
  socket_.recv(reply, zmq::recv_flags::none);

  return parse_response<daphnemodules::ConfigureResponse>(
      reply, daphnemodules::CONFIGURE_FE);
}

daphnemodules::ConfigureCLKsResponse
ControlClient::configure_clks(
    const daphnemodules::ConfigureCLKsRequest& req)
{
  auto bytes = make_envelope_bytes(daphnemodules::CONFIGURE_CLKS, req);
  socket_.send(zmq::buffer(bytes), zmq::send_flags::none);

  zmq::message_t reply;
  socket_.recv(reply, zmq::recv_flags::none);

  return parse_response<daphnemodules::ConfigureCLKsResponse>(
      reply, daphnemodules::CONFIGURE_CLKS);
}

} // namespace daphne::zmq
