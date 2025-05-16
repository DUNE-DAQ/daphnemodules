
cmake_minimum_required(VERSION 3.12)
project(daphnemodules VERSION 2.2.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(appmodel REQUIRED)
find_package(conffwk REQUIRED)
find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(fmt REQUIRED)
find_package(detdataformats REQUIRED)
find_package(fddetdataformats REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(cppzmq            REQUIRED)
find_package(Protobuf          REQUIRED)
find_package(ers               REQUIRED)
find_package(logging           REQUIRED)
find_package(absl              REQUIRED)

#############################################################################

execute_process(
  COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
          --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/proto
          --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/proto
          ${DAPHNE_PROTO_SRC})
file(GLOB DAPHNE_PROTO_CC proto/*.pb.cc)

daq_add_library(
  daphne_core
  src/*.cpp src/daphnemod/*.cpp ${DAPHNE_PROTO_CC}
  LINK_LIBRARIES
    ${Protobuf_LIBRARY} cppzmq ers::ers logging::logging
    appfwk::appfwk fmt::fmt)

daq_add_plugin(DaphneMezzModule duneDAQModule
               LINK_LIBRARIES daphne_core appmodel::appmodel)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen
daq_protobuf_codegen( opmon/*.proto )

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cpp LINK_LIBRARIES appfwk::appfwk fmt::fmt) # Any source files and/or dependent libraries to link in not yet determined
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(DaphneV2ControllerModule duneDAQModule LINK_LIBRARIES daphnemodules appmodel::appmodel fddetdataformats::fddetdataformats )

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

daq_add_unit_test(DaphneInterface_test LINK_LIBRARIES daphnemodules) 

##############################################################################

daq_install()

