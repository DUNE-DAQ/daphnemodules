#include "logging/Logging.hpp" // NOLINT
#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQModule.hpp"
#include "daphnemodules/daphnecontroller/Nljs.hpp"
using namespace dunedaq::appfwk;

int
main()
{
  TLOG() << "Creating Module instances...";
  std::shared_ptr<DAQModule> instdaphnecontroller = make_module("DaphneController", "dummy");
  instdaphnecontroller->init(nlohmann::json{});
  dunedaq::daphnemodules::daphnecontroller::Conf c;

  c.daphne_address = "10.73.137.113";
  c.self_trigger_threshold = 50;
  c.biasctrl = 0;

  dunedaq::daphnemodules::daphnecontroller::ChannelConf chc;
  chc.gain = 2;
  chc.offset = 1050;

  for (size_t i = 1; i < 10; i+=2 ) {
    dunedaq::daphnemodules::daphnecontroller::Channel temp;
    temp.conf = chc;
    temp.id = i;
    c.channels.push_back(temp);
  }
  
  for ( size_t i = 0; i < 2; ++i ) {
    dunedaq::daphnemodules::daphnecontroller::AFE a;
    a.id = i;
    a.v_gain = 2666;

    a.pga.lpf_cut_frequency = 0;
    a.pga.gain = false;

    c.afes.push_back(a);
  }
    
  nlohmann::json j;
  dunedaq::daphnemodules::daphnecontroller::to_json(j,c);
  instdaphnecontroller->execute_command("conf", "ANY", j );
  
  dunedaq::opmonlib::InfoCollector info;
  instdaphnecontroller->get_info(info, 0);

  TLOG() << info.get_collected_infos() ;
  
  return 0;
}

