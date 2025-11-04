/**
 * @file DaphneV3Interface_test.cxx
 *
 * Simple unittest for the daphne v3 interface
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 *
 */


#define BOOST_TEST_MODULE DaphneV3InterfaceTest  // NOLINT
#include <boost/test/included/unit_test.hpp>

#include "DaphneV3Interface.hpp"
#include <iostream>
#include <cstdlib>

using namespace dunedaq::daphnemodules;

BOOST_AUTO_TEST_SUITE(DaphneInterfaceV3_test)

BOOST_AUTO_TEST_CASE(bad_address)
{

 BOOST_CHECK_THROW( dunedaq::daphnemodules::DaphneV3Interface i("bad/address", "v3_unittest"),
		    dunedaq::daphnemodules::InvalidIPAddress );

 BOOST_CHECK_THROW( dunedaq::daphnemodules::DaphneV3Interface i("bad.address:14q4", "v3_unittest"),
		    dunedaq::daphnemodules::InvalidIPAddress );

 BOOST_CHECK_THROW( dunedaq::daphnemodules::DaphneV3Interface i("non.existing.address", "v3_unittest"),
		    dunedaq::daphnemodules::FailedPing );

  BOOST_CHECK_THROW( dunedaq::daphnemodules::DaphneV3Interface i("non.existing.address.with.port:7954", "v3_unittest"),
		     dunedaq::daphnemodules::FailedPing );

}


BOOST_AUTO_TEST_SUITE_END()



// static std::string get_env_or_default(const char* name, const char* def)
// {
//   if (const char* v = std::getenv(name))
//     return v;
//   return def;
// }



// BOOST_AUTO_TEST_CASE(connection_test)
// {
//   std::string ip   = get_env_or_default("DAPHNE_IP", "");
//   std::string port = get_env_or_default("DAPHNE_PORT", "");

//   const auto& args = boost::unit_test::framework::master_test_suite().argv;
//   const int argc   = boost::unit_test::framework::master_test_suite().argc;

//   for (int i = 1; i < argc; ++i) {
//     std::string a = args[i];
//     if (a == "--ip" && i + 1 < argc) ip = args[++i];
//     else if (a == "--port" && i + 1 < argc) port = args[++i];
//   }

//   if (ip.empty()) {
//     BOOST_FAIL("No IP provided (use -- --ip <addr> [--port <num>] or DAPHNE_IP env)");
//   }

//   std::string address = ip;
//   if (!port.empty())
//     address += ":" + port;

//   std::cout << "[TEST] Connecting to " << address << std::endl;

//   std::chrono::milliseconds timeout(3000);
//   DaphneV3Interface iface(address, "v3_unittest", timeout);

//   uint64_t val = 0;
//   bool ok = iface.read_test_register(val);

//   std::cout << "[TEST] Test register value: 0x" << std::hex << val << std::dec << std::endl;

//   BOOST_CHECK_MESSAGE(ok, "Failed to read test register");
//   BOOST_CHECK_EQUAL(val, 0xDEADBEEF);
// }
