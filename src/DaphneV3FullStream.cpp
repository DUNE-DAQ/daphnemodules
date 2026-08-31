/**
 * @file DaphneV3FullStream.cpp
 */

#include "daphnemodules/DaphneV3FullStream.hpp"

#include <array>

namespace dunedaq::daphnemodules {

FullStreamChannelValidation
validate_full_stream_channels(const std::vector<uint32_t>& channels)
{
  if (channels.size() > kDaphneV3FullStreamOutputs) {
    return { FullStreamChannelError::kTooMany, 0 };
  }

  std::array<bool, kDaphneV3ChannelCount> seen{};
  for (const auto channel : channels) {
    if (channel >= kDaphneV3ChannelCount) {
      return { FullStreamChannelError::kOutOfRange, channel };
    }
    if (seen[channel]) {
      return { FullStreamChannelError::kDuplicate, channel };
    }
    seen[channel] = true;
  }

  return {};
}

bool
is_unambiguously_full_stream(const std::vector<uint32_t>& channels)
{
  return !channels.empty();
}

void
append_full_stream_channels(const std::vector<uint32_t>& channels, daphne::ConfigureRequest& request)
{
  for (const auto channel : channels) {
    request.add_full_stream_channels(channel);
  }
}

} // namespace dunedaq::daphnemodules
