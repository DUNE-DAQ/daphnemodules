#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <random>

#include <zmq.hpp>
#include "daphnemodules/daphne_control_high.pb.h"

using namespace daphne;

// ------------- small helpers -------------
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

static inline uint64_t now_ns() {
  using namespace std::chrono;
  return duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count();
}

static inline std::pair<uint64_t,uint64_t> next_ids() {
  static std::mt19937_64 rng{std::random_device{}()};
  static uint64_t seq = 1;
  uint64_t t = now_ns();
  uint64_t task = (t << 16) ^ (rng() & 0xFFFF);
  uint64_t msg  = (t << 1) ^ (++seq);
  task &= ((1ull<<63)-1);
  msg  &= ((1ull<<63)-1);
  return {task, msg};
}

static void usage(const char* prog) {
  std::cerr << "Usage: " << prog << " [--ip <addr>] [--port <num>] [--route <name>]\n"
            << "       " << prog << " [ip] [port]\n"
            << "Env:   DAPHNE_IP, DAPHNE_PORT\n"
            << "Default: 10.73.137.161:40001, route=mezz/0\n";
}

int main(int argc, char** argv) {
  // ---- Defaults
  std::string ip = "10.73.137.161";
  int port = 40001;
  std::string route = "mezz/0";

  // ---- Env
  if (const char* eip = std::getenv("DAPHNE_IP"); eip && *eip) ip = eip;
  if (const char* ep  = std::getenv("DAPHNE_PORT"); ep  && *ep) { try { port = std::stoi(ep); } catch (...) {} }

  // ---- CLI flags (priority over env)
  for (int i = 1; i < argc; ++i) {
    if (!std::strcmp(argv[i], "--help") || !std::strcmp(argv[i], "-h")) {
      usage(argv[0]); return 0;
    }
    if (!std::strcmp(argv[i], "--ip")    && i + 1 < argc) { ip = argv[++i]; continue; }
    if (!std::strcmp(argv[i], "--port")  && i + 1 < argc) { try { port = std::stoi(argv[++i]); } catch (...) {} continue; }
    if (!std::strcmp(argv[i], "--route") && i + 1 < argc) { route = argv[++i]; continue; }
  }
  // ---- Positional args (fallback if flags not used)
  if (argc >= 2 && argv[1][0] != '-') ip = argv[1];
  if (argc >= 3 && argv[2][0] != '-') { try { port = std::stoi(argv[2]); } catch (...) {} }

  const std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);
  std::cerr << "[SMOKE-V2] connecting to " << endpoint << " route=" << route << "\n";

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // ---- Build ConfigureRequest (same payload you used before)
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

  // ---- Wrap in V2 envelope
  ControlEnvelopeV2 req;
  req.set_version(2);
  req.set_dir(DIR_REQUEST);
  req.set_type(MT2_CONFIGURE_FE_REQ);
  req.set_payload(cfg.SerializeAsString());
  {
    auto [task_id, msg_id] = next_ids();
    req.set_task_id(task_id);
    req.set_msg_id(msg_id);
    req.set_timestamp_ns(now_ns());
    if (!route.empty()) req.set_route(route);
  }

  try {
    zmq::context_t ctx(1);
    zmq::socket_t  sock(ctx, zmq::socket_type::dealer);
    sock.set(zmq::sockopt::routing_id, "zmq-config-smoke-v2");
    sock.set(zmq::sockopt::rcvtimeo, 4000);
    sock.set(zmq::sockopt::sndtimeo, 4000);
    sock.connect(endpoint);

    // send
    {
      std::string bytes = req.SerializeAsString();
      zmq::message_t msg(bytes.size());
      std::memcpy(msg.data(), bytes.data(), bytes.size());
      if (!sock.send(msg, zmq::send_flags::none)) {
        std::cerr << "send() timed out\n"; return 2;
      }
    }

    // recv
    auto frames = recv_multipart(sock);
    const zmq::message_t& payload = frames.back();

    ControlEnvelopeV2 rep;
    if (!rep.ParseFromArray(payload.data(), static_cast<int>(payload.size()))) {
      std::cerr << "Failed to parse ControlEnvelopeV2 reply\n"; return 3;
    }

    // validate V2 transport
    bool ok_dir  = (rep.dir()  == DIR_RESPONSE);
    bool ok_type = (rep.type() == MT2_CONFIGURE_FE_RESP);
    bool ok_tid  = (rep.task_id()   == req.task_id());
    bool ok_corr = (rep.correl_id() == req.msg_id());
    if (!ok_dir || !ok_type || !ok_tid || !ok_corr) {
      std::cerr << "Correlation/type mismatch:"
                << "\n  dir:  got " << rep.dir()  << " expect " << DIR_RESPONSE
                << "\n  type: got " << rep.type() << " expect " << MT2_CONFIGURE_FE_RESP
                << "\n  task: got " << rep.task_id()  << " expect " << req.task_id()
                << "\n  corr: got " << rep.correl_id()<< " expect " << req.msg_id()
                << "\n";
      return 4;
    }

    // decode payload
    ConfigureResponse resp;
    if (!resp.ParseFromString(rep.payload())) {
      std::cerr << "Failed to parse ConfigureResponse\n"; return 5;
    }

    std::cout << "[V2 meta]\n";
    std::cout << "  task_id      : " << rep.task_id() << "\n";
    std::cout << "  msg_id       : " << rep.msg_id() << "\n";
    std::cout << "  correl_id    : " << rep.correl_id() << "\n";
    std::cout << "  timestamp_ns : " << rep.timestamp_ns() << "\n";
    if (!rep.route().empty())
      std::cout << "  route        : " << rep.route() << "\n";
    std::cout << "\n";
    std::cout << "Success: " << std::boolalpha << resp.success() << "\n";
    std::cout << "Message:\n" << resp.message() << "\n";

    google::protobuf::ShutdownProtobufLibrary();
    return resp.success() ? 0 : 6;

  } catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
    return 7;
  }
}
