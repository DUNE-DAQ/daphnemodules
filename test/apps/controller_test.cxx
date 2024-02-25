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
  c.daphne_list.push_back("10.73.137.104");
  c.daphne_list.push_back("10.73.137.105");
  c.daphne_list.push_back("10.73.137.107");
  c.daphne_list.push_back("10.73.137.109");
  c.daphne_list.push_back("10.73.137.112");
  c.daphne_list.push_back("10.73.137.113");
  nlohmann::json j;
  dunedaq::daphnemodules::daphnecontroller::to_json(j,c);
  instdaphnecontroller->execute_command("conf", "ANY", j );
return 0;}

