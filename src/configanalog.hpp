#include <logging/Logging.hpp>

namespace dunedaq::daphnemodules {
class confanalog {
public:
    confanalog(OEI& thing) {
         std::string initial_command = "CFG AFE ALL INITIAL";
         cmd (thing, initial_command);
         int nChannels = 40;
         for (int ch = 0; ch < nChannels; ++ch) {
             cmd (thing, "WR OFFSET CH " + std::to_string(ch) + " V 1468", true);
             cmd (thing, "CFG OFFSET CH " + std::to_string(ch) + " GAIN 2", true);
         }
         TLOG() << "Configuring AFE registers 4, 51, 52 and Attenuators";
         int nAFEs = 5;
         for (int AFE = 0; AFE < nAFEs; ++AFE) {
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 52 V 21056", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 4 V 24", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " REG 51 V 16", true);
         cmd (thing, "WR AFE " + std::to_string(AFE) + " VGAIN V 2667", true);

        }
    }
};
}
