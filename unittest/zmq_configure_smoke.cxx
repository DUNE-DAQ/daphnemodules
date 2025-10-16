#include <iostream>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "daphnemodules/daphne_control_high.pb.h"   // brings ControlEnvelope, Configure*
using namespace daphne;

static void send_and_recv(zmq::socket_t& sock, const ControlEnvelope& env, ControlEnvelope& reply_env)
{
  std::string out = env.SerializeAsString();
  zmq::message_t msg(out.size());
  memcpy(msg.data(), out.data(), out.size());
  sock.send(msg, zmq::send_flags::none);

  // Compatible with ROUTER: REQ receives a single frame (server sends [id][payload])
  zmq::message_t rep;
  sock.recv(rep, zmq::recv_flags::none);
  if (!reply_env.ParseFromArray(rep.data(), (int)rep.size())) {
    throw std::runtime_error("Failed to parse reply envelope");
  }
}

int main(int argc, char** argv)
{
  // ---- Hardcoded target ----
  const std::string host = (argc > 1) ? argv[1] : "127.0.0.1";
  const int         port = (argc > 2) ? std::stoi(argv[2]) : 9000; // match your server’s port
  const std::string endpoint = "tcp://" + host + ":" + std::to_string(port);
  std::cerr << "[SMOKE] connecting to " << endpoint << "\n";

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // ---- Build a minimal-but-realistic ConfigureRequest ----
  ConfigureRequest cfg;
  cfg.set_daphne_address(host);
  cfg.set_slot(0);
  cfg.set_timeout_ms(500);
  cfg.set_biasctrl(1300);

  cfg.set_self_trigger_threshold(0x1F40);   // 8000
  cfg.set_self_trigger_xcorr(0x68);
  cfg.set_tp_conf(0x0010DB35);
  cfg.set_compensator(0xFFFFFFFFFFull);
  cfg.set_inverters(0xFF00000000ull);

  // 40 channels with a fixed pedestal and zero trim
  for (uint32_t ch = 0; ch < 40; ++ch) {
    auto* c = cfg.add_channels();
    c->set_id(ch);
    c->set_trim(0);
    c->set_offset(2275);
    c->set_gain(1);
  }

  // 5 AFEs with typical analog settings
  for (uint32_t afe = 0; afe < 5; ++afe) {
    auto* a = cfg.add_afes();
    a->set_id(afe);
    a->set_attenuators(1600);
    a->set_v_bias(0);

    a->mutable_adc()->set_resolution(true);
    a->mutable_adc()->set_output_format(true);
    a->mutable_adc()->set_sb_first(false);

    a->mutable_pga()->set_lpf_cut_frequency(4);   // 10 MHz code in your map
    a->mutable_pga()->set_integrator_disable(true);
    a->mutable_pga()->set_gain(0);

    a->mutable_lna()->set_clamp(0);
    a->mutable_lna()->set_gain(2);
    a->mutable_lna()->set_integrator_disable(true);
  }

  // ---- Wrap in envelope
  ControlEnvelope env;
  env.set_type(CONFIGURE_FE);
  env.set_payload(cfg.SerializeAsString());

  // ---- ZMQ REQ client (compatible with your ROUTER server)
  zmq::context_t ctx(1);
  zmq::socket_t  sock(ctx, zmq::socket_type::req);
  sock.connect(endpoint);

  ControlEnvelope reply_env;
  try {
    send_and_recv(sock, env, reply_env);
  } catch (const std::exception& e) {
    std::cerr << "send/recv error: " << e.what() << "\n";
    return 2;
  }

  if (reply_env.type() != CONFIGURE_FE) {
    std::cerr << "Unexpected reply type: " << reply_env.type() << "\n";
    return 3;
  }

  ConfigureResponse resp;
  if (!resp.ParseFromString(reply_env.payload())) {
    std::cerr << "Failed to parse ConfigureResponse\n";
    return 4;
  }

  std::cout << "Success: " << std::boolalpha << resp.success() << "\n";
  std::cout << "Message:\n" << resp.message() << "\n";

  google::protobuf::ShutdownProtobufLibrary();
  return resp.success() ? 0 : 5;
}
