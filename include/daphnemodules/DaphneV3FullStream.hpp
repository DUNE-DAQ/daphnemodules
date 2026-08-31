/**
 * @file DaphneV3FullStream.hpp
 *
 * Validation and protobuf serialization helpers for DAPHNE V3 full-stream
 * channel selections.
 */

#ifndef DAPHNEMODULES_INCLUDE_DAPHNEMODULES_DAPHNEV3FULLSTREAM_HPP_
#define DAPHNEMODULES_INCLUDE_DAPHNEMODULES_DAPHNEV3FULLSTREAM_HPP_

#include "daphnemodules/daphne_control_high.pb.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dunedaq::daphnemodules {

constexpr std::size_t kDaphneV3FullStreamOutputs = 32;
constexpr uint32_t kDaphneV3ChannelCount = 40;

enum class FullStreamChannelError
{
  kNone,
  kTooMany,
  kOutOfRange,
  kDuplicate
};

struct FullStreamChannelValidation
{
  FullStreamChannelError error = FullStreamChannelError::kNone;
  uint32_t channel = 0;

  explicit operator bool() const { return error == FullStreamChannelError::kNone; }
};

FullStreamChannelValidation
validate_full_stream_channels(const std::vector<uint32_t>& channels);

bool
is_unambiguously_full_stream(const std::vector<uint32_t>& channels);

void
append_full_stream_channels(const std::vector<uint32_t>& channels, daphne::ConfigureRequest& request);

} // namespace dunedaq::daphnemodules

#endif // DAPHNEMODULES_INCLUDE_DAPHNEMODULES_DAPHNEV3FULLSTREAM_HPP_
