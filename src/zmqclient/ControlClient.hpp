#ifndef DAPHNE_CONTROL_CLIENT_HPP
#define DAPHNE_CONTROL_CLIENT_HPP
#pragma once

#include <string>
#include <cstdint>
#include <zmq.hpp>

#include "daphnemodules/daphne_control_envelope.pb.h"
#include "daphnemodules/daphne_control_high.pb.h"

namespace daphne::zmq {

/**
 * @brief Thin RAII wrapper that sends Configure* protobuf requests
 *        and returns the corresponding responses.
 *
 * Lifetime:
 *   - holds a ref to the external ZeroMQ context (no duplication)
 *   - owns a DEALER/REQ socket
 */
class ControlClient
{
public:
  using Milliseconds = std::chrono::milliseconds;

  ControlClient(zmq::context_t& ctx,
                std::string_view ip,
                uint16_t          port,
                Milliseconds      timeout = Milliseconds{500});

  /** Send an already-filled ConfigureRequest and wait for reply. */
  daphnemodules::ConfigureResponse
  configure(const daphnemodules::ConfigureRequest& req);

  /** Same for ConfigureCLKsRequest */
  daphnemodules::ConfigureCLKsResponse
  configure_clks(const daphnemodules::ConfigureCLKsRequest& req);

private:
  zmq::socket_t socket_;
};

} // namespace daphne::zmq
#endif /* DAPHNE_CONTROL_CLIENT_HPP */
