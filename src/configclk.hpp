#include <chrono>
#include <thread>


namespace dunedaq::daphnemodules {
class configclk {
public:
    configclk(OEI& thing,int ep) {
        using namespace std::this_thread; 
        using namespace std::chrono; 
        uint8_t USE_ENDPOINT = 1;
        uint8_t EDGE_SELECT = 0;
        uint8_t TIMING_GROUP = 0;
        uint8_t ENDPOINT_ADDRESS = 0;
        thing.write(0x4001, {USE_ENDPOINT});
        thing.write(0x3000, {0x002081 + uint64_t(0x400000 * ep)});
        thing.write(0x3001, {0b11111111});
        thing.write(0x4003, {1234});
        sleep_for(1s);
        thing.write(0x4002, {1234});
        sleep_for(1s);
        thing.write(0x2001, {1234});
        sleep_for(1s);
    }
};
}
