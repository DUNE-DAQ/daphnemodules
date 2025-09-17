#pragma once
#include <string>
#include <fmt/format.h>

namespace dunedaq::daphnemodules::mezz {

class CommandBuilder
{
public:
  static std::string build_bias_cmd(int afe, int mv)
  {
    return fmt::format("WR BIASSET AFE {} V {}", afe, mv);
  }

  static std::string build_vgain_cmd(int afe, int vgain)
  {
    return fmt::format("WR AFE {} VGAIN V {}", afe, vgain);
  }

};

} 
