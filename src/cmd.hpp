#include <chrono>
#include <thread>
#include <logging/Logging.hpp>

namespace dunedaq::daphnemodules {
class cmd {
public:
    cmd(OEI& thing, const std::string& CmdString, bool get_response = true) {
        std::vector<uint64_t> CmdByteList;
        for (char ch : CmdString) {
            CmdByteList.push_back(static_cast<uint64_t>(ch));
        }
        CmdByteList.push_back(0x0d);

        for (size_t i = 0; i < (CmdByteList.size() + 49) / 50; ++i) {
            std::vector<uint64_t> part(CmdByteList.begin() + i*50, CmdByteList.begin() + std::min((i+1)*50, CmdByteList.size()));
            thing.writef(0x90000000, part);
        }

        std::string ResString = "";
        if (get_response) {
            int more = 40;
            while (more > 0) {
                auto ResByteList = thing.readf(0x90000000, 50);
                for (size_t i = 2; i < ResByteList.size(); ++i) {
                    if (ResByteList[i] == 255) {
                        break;
                    } else if (ResByteList[i] == 1) {
                        ResString += "[START]";
                    } else if (ResByteList[i] == 2) {
                        ResString += "[RESULT]";
                    } else if (ResByteList[i] == 3) {
                        ResString += "[END]";
                    } else if (isprint(static_cast<int>(ResByteList[i]))) {
                        more = 40;
                        ResString += static_cast<char>(ResByteList[i]);
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                --more;
            }
            ResString += '\0';
        }
    }
};
}
