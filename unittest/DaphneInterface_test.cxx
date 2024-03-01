/**
 * @file DaphneInterface_test.cxx
 *
 * This file provides a skeleton off of which developers can write
 * unit tests for their package. The file is meant to be renamed as
 * well as edited (where editing includes replacing this comment with
 * an actual description of the unit test suite)
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#define BOOST_TEST_MODULE DaphneInterface_test // NOLINT

#include "boost/test/unit_test.hpp"
#include "DaphneInterface.hpp"

BOOST_AUTO_TEST_SUITE(DaphneInterface_test)

BOOST_AUTO_TEST_CASE(Construction)
{

  BOOST_CHECK_THROW( dunedaq::daphnemodules::DaphneInterface i("non.exising.ip", 1000),
		     dunedaq::daphnemodules::InvalidIPAddress );

}

BOOST_AUTO_TEST_SUITE_END()
