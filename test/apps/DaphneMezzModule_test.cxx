#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>      // getenv
#include <cstring>      // std::strcmp
#include <zmq.hpp>

#include "daphnemodules/daphne_control_high.pb.h"
using namespace daphne;

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

static void usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [--ip <addr>] [--port <num>]\n"
            << "       " << prog << " [ip] [port]\n"
            << "Env:   DAPHNE_IP, DAPHNE_PORT\n"
            << "Default: 10.73.137.161:9000\n";
}

int main(int argc, char** argv) {
  // ---- Defaults
  std::string ip = "10.73.137.161";
  int port = 9000;

  // ---- Env
  if (const char* eip = std::getenv("DAPHNE_IP"); eip && *eip) ip = eip;
  if (const char* ep  = std::getenv("DAPHNE_PORT"); ep  && *ep) { try { port = std::stoi(ep); } catch (...) {} }

  // ---- CLI flags (priority over env)
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
      usage(argv[0]); return 0;
    }
    if (!std::strcmp(argv[i], "--ip") && i + 1 < argc) { ip = argv[++i]; continue; }
    if (!std::strcmp(argv[i], "--port") && i + 1 < argc) { try { port = std::stoi(argv[++i]); } catch (...) {} continue; }
  }
  // ---- Positional args (fallback if flags not used)
  if (argc >= 2 && argv[1][0] != '-') ip = argv[1];
  if (argc >= 3 && argv[2][0] != '-') { try { port = std::stoi(argv[2]); } catch (...) {} }

  const std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);
  std::cerr << "[SMOKE] connecting to " << endpoint << "\n";

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // ---- Build ConfigureRequest (matches server)
  ConfigureRequest cfg;
  cfg.set_daphne_address(ip);
  cfg.set_slot(0);
  cfg.set_timeout_ms(500);
  cfg.set_biasctrl(1300);
  cfg.set_self_trigger_threshold(0x1F40);
  cfg.set_self_trigger_xcorr(0x68);
  cfg.set_tp_conf(0x0010DB35);
  cfg.set_compensator(0xFFFFFFFFFFull);
  cfg.set_inverters(0xFF00000000ull);

  for (uint32_t ch = 0; ch < 40; ++ch) {
    auto* c = cfg.add_channels();
    c->set_id(ch);
    c->set_trim(0);
    c->set_offset(2275);
    c->set_gain(1);
  }
  for (uint32_t afe = 0; afe < 5; ++afe) {
    auto* a = cfg.add_afes();
    a->set_id(afe);
    a->set_attenuators(1600);
    a->set_v_bias(0);
    a->mutable_adc()->set_resolution(true);
    a->mutable_adc()->set_output_format(true);
    a->mutable_adc()->set_sb_first(false);
    a->mutable_pga()->set_lpf_cut_frequency(4);
    a->mutable_pga()->set_integrator_disable(true);
    a->mutable_pga()->set_gain(0);
    a->mutable_lna()->set_clamp(0);
    a->mutable_lna()->set_gain(2);
    a->mutable_lna()->set_integrator_disable(true);
  }

  ControlEnvelope env;
  env.set_type(CONFIGURE_FE);
  env.set_payload(cfg.SerializeAsString());

  try {
    zmq::context_t ctx(1);
    zmq::socket_t  sock(ctx, zmq::socket_type::dealer);
    sock.set(zmq::sockopt::routing_id, "zmq-config-smoke");
    sock.set(zmq::sockopt::rcvtimeo, 4000);
    sock.set(zmq::sockopt::sndtimeo, 4000);
    sock.connect(endpoint);

    std::string out = env.SerializeAsString();
    zmq::message_t msg(out.size());
    std::memcpy(msg.data(), out.data(), out.size());
    if (!sock.send(msg, zmq::send_flags::none)) {
      std::cerr << "send() timed out\n"; return 2;
    }

    auto frames = recv_multipart(sock);
    const zmq::message_t& payload = frames.back();

    ControlEnvelope reply_env;
    if (!reply_env.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
      std::cerr << "Failed to parse reply envelope\n"; return 3;
    }
    if (reply_env.type() != CONFIGURE_FE) {
      std::cerr << "Unexpected reply type: " << reply_env.type() << "\n"; return 4;
    }

    ConfigureResponse resp;
    if (!resp.ParseFromString(reply_env.payload())) {
      std::cerr << "Failed to parse ConfigureResponse\n"; return 5;
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
