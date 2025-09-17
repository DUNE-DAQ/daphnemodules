#include "DaphneMezzModule.hpp"
#include "logging/Logging.hpp"
#include "daphnemodules/daphne_control_high.pb.h"
#include "daphnemodules/daphne_control_envelope.pb.h"

#include <zmq.hpp>

namespace dunedaq::daphnemodules {

DaphneMezzModule::DaphneMezzModule(const std::string& name)
  : appfwk::DAQModule(name)
{
  register_command("conf",  &DaphneMezzModule::do_conf);
  register_command("start", &DaphneMezzModule::do_start);
  register_command("scrap", &DaphneMezzModule::do_scrap);
}

void DaphneMezzModule::init(std::shared_ptr<appfwk::ConfigurationManager> cfg)
{
}


void DaphneMezzModule::do_conf(const CommandData_t&)
{
  using namespace daphne;

  // Step 1: Build the ConfigureRequest
  ConfigureRequest req;
  req.set_daphne_address("192.168.0.10");
  req.set_slot(1);
  req.set_timeout_ms(500);
  req.set_biasctrl(800);
  req.set_self_trigger_threshold(0);
  req.set_self_trigger_xcorr(0);
  req.set_tp_conf(0);
  req.set_compensator(0);
  req.set_inverters(0);

  auto* ch = req.add_channels();
  ch->set_id(0);
  ch->set_trim(64);
  ch->set_offset(1000);
  ch->set_gain(1);

  auto* afe = req.add_afes();
  afe->set_id(0);
  afe->set_v_gain(1);
  afe->set_v_bias(800);
  afe->mutable_adc()->set_resolution(true);
  afe->mutable_adc()->set_output_format(false);
  afe->mutable_adc()->set_sb_first(true);
  afe->mutable_pga()->set_lpf_cut_frequency(5);
  afe->mutable_pga()->set_integrator_disable(false);
  afe->mutable_pga()->set_gain(true);
  afe->mutable_lna()->set_clamp(2);
  afe->mutable_lna()->set_gain(2);
  afe->mutable_lna()->set_integrator_disable(false);

  // Step 2: Wrap in Envelope
  ControlEnvelope env;
  env.set_type(CONFIGURE_FE);
  env.set_payload(req.SerializeAsString());

  // Step 3: ZMQ send/recv
  zmq::context_t context(1);
  zmq::socket_t socket(context, zmq::socket_type::req);
  socket.connect("tcp://193.206.157.36:9000");

  std::string out_str = env.SerializeAsString();
  zmq::message_t message(out_str.size());
  memcpy(message.data(), out_str.data(), out_str.size());
  socket.send(message, zmq::send_flags::none);

  zmq::message_t reply;
  socket.recv(reply, zmq::recv_flags::none);

  ControlEnvelope response_env;
  response_env.ParseFromArray(reply.data(), reply.size());

  TLOG() << "Received response of type: " << response_env.type();

  if (response_env.type() == CONFIGURE_FE) {
    ConfigureResponse resp;
    resp.ParseFromString(response_env.payload());
    TLOG() << "Success: " << resp.success();
    TLOG() << "Message: " << resp.message();
  } else {
    TLOG() << "Unexpected message type: " << response_env.type();
  }
}

void DaphneMezzModule::do_start(const CommandData_t& )  { /* nothing yet */ }
void DaphneMezzModule::do_scrap(const CommandData_t&)  { m_iface.reset(); }

} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneMezzModule)
