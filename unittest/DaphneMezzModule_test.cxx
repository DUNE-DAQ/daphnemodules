#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>      // getenv
#include <zmq.hpp>

// This file already defines ControlEnvelope + MessageType (CONFIGURE_FE)
#include "daphnemodules/daphne_control_high.pb.h"

using namespace daphne;

// Receive all frames from ROUTER; last one is the payload
static std::vector<zmq::message_t> recv_multipart(zmq::socket_t& s) {
  std::vector<zmq::message_t> frames;
  while (true) {
    zmq::message_t part;
    auto ok = s.recv(part, zmq::recv_flags::none);
    if (!ok || *ok <= 0) throw std::runtime_error("No response from slow controller");
    frames.emplace_back(std::move(part));
    if (!s.get(zmq::sockopt::rcvmore)) break;
  }
  return frames;
}

int main(int argc, char** argv) {
  // ---- Target host/port (defaults match your server) -----------------------
  const std::string host = (argc > 1) ? argv[1] : "10.73.137.161";
  int port = (argc > 2) ? std::stoi(argv[2]) : 9000;
  if (const char* p = std::getenv("DAPHNE_PORT")) {
    try { port = std::stoi(p); } catch (...) {}
  }
  const std::string endpoint = "tcp://" + host + ":" + std::to_string(port);
  std::cerr << "[SMOKE] connecting to " << endpoint << "\n";

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // ---- Build a realistic ConfigureRequest (matches your server) ------------
  ConfigureRequest cfg;
  cfg.set_daphne_address(host);
  cfg.set_slot(0);
  cfg.set_timeout_ms(500);
  cfg.set_biasctrl(1300);

  // Your trigger/TP constants
  cfg.set_self_trigger_threshold(0x1F40);       // 8000
  cfg.set_self_trigger_xcorr(0x68);
  cfg.set_tp_conf(0x0010DB35);
  cfg.set_compensator(0xFFFFFFFFFFull);         // 48-bit in uint64
  cfg.set_inverters(0xFF00000000ull);           // 48-bit in uint64

  // 40 channels: trim=0, offset=2275, gain=1
  for (uint32_t ch = 0; ch < 40; ++ch) {
    auto* c = cfg.add_channels();
    c->set_id(ch);
    c->set_trim(0);
    c->set_offset(2275);
    c->set_gain(1);
  }

  // 5 AFEs with the analog chain you use in Python
  for (uint32_t afe = 0; afe < 5; ++afe) {
    auto* a = cfg.add_afes();
    a->set_id(afe);
    a->set_attenuators(1600);  // “VGAIN” DAC in your server naming
    a->set_v_bias(0);

    auto* adc = a->mutable_adc();
    adc->set_resolution(true);
    adc->set_output_format(true);
    adc->set_sb_first(false);

    auto* pga = a->mutable_pga();
    pga->set_lpf_cut_frequency(4);     // code for 10 MHz in your map
    pga->set_integrator_disable(true);
    pga->set_gain(0);

    auto* lna = a->mutable_lna();
    lna->set_clamp(0);
    lna->set_gain(2);
    lna->set_integrator_disable(true);
  }

  // Envelope (same .proto as server; enum = MessageType)
  ControlEnvelope env;
  env.set_type(CONFIGURE_FE);
  env.set_payload(cfg.SerializeAsString());

  try {
    // ---- DEALER socket works with your ROUTER + identity for nice logs ----
    zmq::context_t ctx(1);
    zmq::socket_t  sock(ctx, zmq::socket_type::dealer);
    sock.set(zmq::sockopt::routing_id, "zmq-config-smoke");
    // timeouts so we never hang forever
    sock.set(zmq::sockopt::rcvtimeo, 4000);  // 4s
    sock.set(zmq::sockopt::sndtimeo, 4000);  // 4s
    sock.connect(endpoint);

    // Send single-frame request (server doesn’t require an empty delimiter)
    std::string out = env.SerializeAsString();
    zmq::message_t msg(out.size());
    std::memcpy(msg.data(), out.data(), out.size());
    if (!sock.send(msg, zmq::send_flags::none)) {
      std::cerr << "send() timed out\n";
      return 2;
    }

    // Receive multipart reply: ROUTER sends [id][payload]; DEALER sees both
    auto frames = recv_multipart(sock);
    const zmq::message_t& payload = frames.back();

    ControlEnvelope reply_env;
    if (!reply_env.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
      std::cerr << "Failed to parse reply envelope\n";
      return 3;
    }
    if (reply_env.type() != CONFIGURE_FE) {
      std::cerr << "Unexpected reply type: " << reply_env.type() << "\n";
      return 4;
    }

    ConfigureResponse resp;
    if (!resp.ParseFromString(reply_env.payload())) {
      std::cerr << "Failed to parse ConfigureResponse\n";
      return 5;
    }

    std::cout << "Success: " << std::boolalpha << resp.success() << "\n";
    std::cout << "Message:\n" << resp.message() << "\n";
    google::protobuf::ShutdownProtobufLibrary();
    return resp.success() ? 0 : 6;

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 7;
  }
}
