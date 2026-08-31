/**
 * @file DaphneV3FullStream_test.cxx
 */

#define BOOST_TEST_MODULE DaphneV3FullStream_test // NOLINT

#include "boost/test/included/unit_test.hpp"
#include "daphnemodules/DaphneV3FullStream.hpp"

#include <cstdint>
#include <numeric>
#include <vector>

using namespace dunedaq::daphnemodules;

BOOST_AUTO_TEST_SUITE(DaphneV3FullStream_test)

BOOST_AUTO_TEST_CASE(validates_hardware_limits)
{
  std::vector<uint32_t> maximum(kDaphneV3FullStreamOutputs);
  std::iota(maximum.begin(), maximum.end(), 0);
  BOOST_TEST(static_cast<bool>(validate_full_stream_channels(maximum)));

  auto too_many = maximum;
  too_many.push_back(32);
  BOOST_TEST(static_cast<int>(validate_full_stream_channels(too_many).error) ==
             static_cast<int>(FullStreamChannelError::kTooMany));

  const auto out_of_range = validate_full_stream_channels({ 0, 39, 40 });
  BOOST_TEST(static_cast<int>(out_of_range.error) == static_cast<int>(FullStreamChannelError::kOutOfRange));
  BOOST_TEST(out_of_range.channel == 40U);

  const auto duplicate = validate_full_stream_channels({ 8, 7, 8 });
  BOOST_TEST(static_cast<int>(duplicate.error) == static_cast<int>(FullStreamChannelError::kDuplicate));
  BOOST_TEST(duplicate.channel == 8U);
}

BOOST_AUTO_TEST_CASE(serializes_channels_in_requested_order)
{
  const std::vector<uint32_t> channels{ 39, 0, 16, 7 };
  daphne::ConfigureRequest request;

  append_full_stream_channels(channels, request);

  BOOST_REQUIRE_EQUAL(request.full_stream_channels_size(), channels.size());
  for (std::size_t index = 0; index < channels.size(); ++index) {
    BOOST_TEST(request.full_stream_channels(static_cast<int>(index)) == channels[index]);
  }
}

BOOST_AUTO_TEST_CASE(only_nonempty_selection_unambiguously_identifies_full_stream)
{
  BOOST_TEST(!is_unambiguously_full_stream({}));
  BOOST_TEST(is_unambiguously_full_stream({ 0 }));
}

BOOST_AUTO_TEST_SUITE_END()
