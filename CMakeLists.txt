
cmake_minimum_required(VERSION 3.12)
project(daphnemodules VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()


find_package(appfwk REQUIRED)
find_package(opmonlib REQUIRED)
find_package(Boost COMPONENTS unit_test_framework REQUIRED)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_codegen

daq_codegen(daphnecontroller.jsonnet TEMPLATES Structs.hpp.j2 Nljs.hpp.j2)
daq_codegen(daphnecontrollerinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_library

daq_add_library( *.cpp LINK_LIBRARIES appfwk::appfwk) # Any source files and/or dependent libraries to link in not yet determined
##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_plugin

daq_add_plugin(DaphneController duneDAQModule LINK_LIBRARIES daphnemodules)

##############################################################################


# See https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#daq_add_unit_test

daq_add_unit_test(DaphneInterface_test LINK_LIBRARIES daphnemodules) 

##############################################################################
daq_add_application(daphne_controller_test controller_test.cxx TEST LINK_LIBRARIES daphnemodules)


daq_install()

