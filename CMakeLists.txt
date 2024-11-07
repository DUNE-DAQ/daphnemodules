
cmake_minimum_required(VERSION 3.12)
project(daphnemodules VERSION 1.5.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen
daq_protobuf_codegen( opmon/*.proto )

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cpp LINK_LIBRARIES appfwk::appfwk fmt::fmt) # Any source files and/or dependent libraries to link in not yet determined
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(DaphneController duneDAQModule LINK_LIBRARIES daphnemodules)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

daq_add_unit_test(DaphneInterface_test LINK_LIBRARIES daphnemodules) 

##############################################################################
# daq_add_application(daphne_controller_test controller_test.cxx TEST LINK_LIBRARIES daphnemodules)

daq_install()

