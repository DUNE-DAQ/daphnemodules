cmake_minimum_required(VERSION 3.12)
project(daphnemodules VERSION 2.2.1)

find_package(daq-cmake         REQUIRED)
daq_setup_environment()

find_package(appmodel          REQUIRED)
find_package(appfwk            REQUIRED)
find_package(conffwk           REQUIRED)
find_package(detdataformats    REQUIRED)
find_package(fddetdataformats  REQUIRED)
find_package(opmonlib          REQUIRED)
find_package(cppzmq            REQUIRED)
find_package(fmt               REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

daq_protobuf_codegen(*.proto  opmon/*.proto)

daq_add_library(
    *.cpp daphnemodules/*.cpp 
    daphnemodules/zmqclient/*.cpp
    LINK_LIBRARIES cppzmq appfwk::appfwk fmt::fmt )

daq_add_plugin(DaphneV3ControllerModule duneDAQModule
    LINK_LIBRARIES daphnemodules appmodel::appmodel)

daq_add_plugin(DaphneV2ControllerModule  duneDAQModule
    LINK_LIBRARIES daphnemodules appmodel::appmodel
    fddetdataformats::fddetdataformats)

daq_add_unit_test(DaphneInterface_test
    LINK_LIBRARIES daphnemodules Boost::unit_test_framework)

daq_add_unit_test(DaphneMezzModule_test
    LINK_LIBRARIES daphnemodules Boost::unit_test_framework)

daq_install()
