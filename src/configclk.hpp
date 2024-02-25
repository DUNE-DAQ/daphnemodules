#include <chrono>
#include <thread>


namespace dunedaq::daphnemodules {
class config {
public:
    config(OEI& thing,int ep) {
        using namespace std::this_thread; 
        using namespace std::chrono; 
        printf("DAPHNE firmware version %0X\n", thing.read(0x9000, 1)[0]);
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
        printf("AFE automatic alignment done, should read 0x1F: %0X\n", thing.read(0x2002, 1)[0]);
        printf("AFE0 Error Count = %0X\n", thing.read(0x2010, 1)[0]);
        printf("AFE1 Error Count = %0X\n", thing.read(0x2011, 1)[0]);
        printf("AFE2 Error Count = %0X\n", thing.read(0x2012, 1)[0]);
        printf("AFE3 Error Count = %0X\n", thing.read(0x2013, 1)[0]);
        printf("AFE4 Error Count = %0X\n", thing.read(0x2014, 1)[0]);
        printf("Crate number = %0X\n", thing.read(0x3000, 1)[0]);
    }
};
}
