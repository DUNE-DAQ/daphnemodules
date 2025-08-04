/**
 * Integration test for DaphneMezzModule using protobuf and ZeroMQ.
 * Loads configuration from a JSON file passed as: -- --json path/to/file.json
 */

#define BOOST_TEST_MODULE DaphneMezzModule_test 

#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>

#include "daphnemodules/daphne_control_high.pb.h"
#include "daphnemodules/daphne_control_envelope.pb.h"

#include <zmq.hpp>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;
using namespace daphne;

std::string json_config_path;

// Struct to parse custom --json argument
struct ArgsParser {
  ArgsParser() {
    auto& argc = boost::unit_test::framework::master_test_suite().argc;
    auto& argv = boost::unit_test::framework::master_test_suite().argv;

    for (int i = 1; i < argc - 1; ++i) {
      if (std::string(argv[i]) == "--json") {
        json_config_path = argv[i + 1];
      }
    }

    if (json_config_path.empty()) {
      throw std::runtime_error("Missing required argument: --json path/to/config.json");
    }
  }
};

BOOST_GLOBAL_FIXTURE(ArgsParser);

BOOST_AUTO_TEST_SUITE(DaphneMezzModule_test)

BOOST_AUTO_TEST_CASE(ConfigureFromJson)
{
  std::ifstream jfile(json_config_path);
  BOOST_REQUIRE_MESSAGE(jfile, "Cannot open JSON file: " << json_config_path);

  json cfg;
  jfile >> cfg;

  BOOST_REQUIRE_MESSAGE(!cfg.empty(), "JSON config is empty");

  const auto ip_it = cfg.begin();
  const std::string ip_key = ip_it.key();
  const json device = ip_it.value();

  BOOST_TEST_MESSAGE("Sending configuration to IP: " + ip_key);

  std::cerr << "DEBUG slot = " << device["slot"] << "\n";
  std::cerr << "DEBUG bias_ctrl = " << device["bias_ctrl"] << "\n";
  std::cerr << "DEBUG self_trigger_threshold = " << device["self_trigger_threshold"] << "\n";
  std::cerr << "DEBUG self_trigger_xcorr = " << device["self_trigger_xcorr"] << "\n";
  std::cerr << "DEBUG tp_conf = " << device["tp_conf"] << "\n";
  std::cerr << "DEBUG compensator = " << device["compensator"] << "\n";
  std::cerr << "DEBUG inverter = " << device["inverter"] << "\n";

  ConfigureRequest req;
  req.set_daphne_address(ip_key);
  req.set_slot(device["slot"]);
  req.set_timeout_ms(500);
  req.set_biasctrl(device["bias_ctrl"]);
  req.set_self_trigger_threshold(device["self_trigger_threshold"]);
  req.set_self_trigger_xcorr(device["self_trigger_xcorr"]);
  req.set_tp_conf(device["tp_conf"]);
  req.set_compensator(device["compensator"]);
  req.set_inverters(device["inverter"]);

  const auto& ch_ids     = device["channel_analog_conf"]["ids"];
  const auto& ch_gains   = device["channel_analog_conf"]["gains"];
  const auto& ch_offsets = device["channel_analog_conf"]["offsets"];
  const auto& ch_trims   = device["channel_analog_conf"]["trims"];

  for (size_t i = 0; i < ch_ids.size(); ++i) {
    auto* ch = req.add_channels();
    ch->set_id(ch_ids[i]);
    ch->set_trim(ch_trims[i]);
    ch->set_offset(ch_offsets[i]);
    ch->set_gain(ch_gains[i]);
  }

  const auto& afes        = device["afes"];
  const auto& afe_ids     = afes["ids"];
  const auto& afe_atten   = afes["attenuators"];
  const auto& afe_vbias   = afes["v_biases"];
  const auto& adc         = afes["adcs"];
  const auto& pga         = afes["pgas"];
  const auto& lna         = afes["lnas"];

  for (size_t i = 0; i < afe_ids.size(); ++i) {
    std::cerr << "DEBUG afe[" << i << "] id=" << afe_ids[i] << "\n";
    std::cerr << "  attenuator=" << afe_atten[i] << "\n";
    std::cerr << "  vbias=" << afe_vbias[i] << "\n";
    std::cerr << "  adc.resolution=" << adc["resolution"][i] << "\n";
    std::cerr << "  adc.output_format=" << adc["output_format"][i] << "\n";
    std::cerr << "  adc.SB_first=" << adc["SB_first"][i] << "\n";
    std::cerr << "  pga.lpf_cut_frequency=" << pga["lpf_cut_frequency"][i] << "\n";
    std::cerr << "  pga.integrator_disable=" << pga["integrator_disable"][i] << "\n";
    std::cerr << "  pga.gain=" << pga["gain"][i] << "\n";
    std::cerr << "  lna.clamp=" << lna["clamp"][i] << "\n";
    std::cerr << "  lna.gain=" << lna["gain"][i] << "\n";
    std::cerr << "  lna.integrator_disable=" << lna["integrator_disable"][i] << "\n";
      auto* afe = req.add_afes();
    afe->set_id(afe_ids[i]);
    afe->set_v_gain(afe_atten[i].get<bool>());
    afe->set_v_bias(afe_vbias[i]);

    auto* adc_conf = afe->mutable_adc();
    adc_conf->set_resolution(adc["resolution"][i]);
    adc_conf->set_output_format(adc["output_format"][i]);
    adc_conf->set_sb_first(adc["SB_first"][i].get<bool>());

    auto* pga_conf = afe->mutable_pga();
    pga_conf->set_lpf_cut_frequency(pga["lpf_cut_frequency"][i]);
    pga_conf->set_integrator_disable(pga["integrator_disable"][i].get<bool>());
    pga_conf->set_gain(pga["gain"][i]);

    auto* lna_conf = afe->mutable_lna();
    lna_conf->set_clamp(lna["clamp"][i].get<bool>());
    lna_conf->set_gain(lna["gain"][i]);
    lna_conf->set_integrator_disable(lna["integrator_disable"][i].get<bool>());
  }

  ControlEnvelope env;
  env.set_type(CONFIGURE_FE);
  env.set_payload(req.SerializeAsString());

  zmq::context_t ctx(1);
  zmq::socket_t sock(ctx, zmq::socket_type::req);
  sock.connect("tcp://" + ip_key + ":9000");

  std::string out_str = env.SerializeAsString();
  zmq::message_t message(out_str.size());
  memcpy(message.data(), out_str.data(), out_str.size());
  sock.send(message, zmq::send_flags::none);

  zmq::message_t reply;
  auto result = sock.recv(reply, zmq::recv_flags::none);
  BOOST_REQUIRE_MESSAGE(result && *result > 0, "No response received from mezzanine");

  ControlEnvelope response_env;
  BOOST_REQUIRE_MESSAGE(response_env.ParseFromArray(reply.data(), reply.size()), "Failed to parse response envelope");

  BOOST_CHECK_EQUAL(response_env.type(), CONFIGURE_FE);

  ConfigureResponse response;
  BOOST_REQUIRE_MESSAGE(response.ParseFromString(response_env.payload()), "Failed to parse ConfigureResponse");

  BOOST_CHECK_MESSAGE(response.success(), "Mezzanine reported failure");
  BOOST_TEST_MESSAGE("Success: " + std::string(response.success() ? "true" : "false"));
  BOOST_TEST_MESSAGE("Message: " + response.message());
}

BOOST_AUTO_TEST_SUITE_END()
