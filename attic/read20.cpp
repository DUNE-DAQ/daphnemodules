#include "oei.hpp"

int main() {
    OEI thing("10.73.137.104");
    std::vector<uint64_t> data;
    data.push_back(1234);
    thing.write(0x2000, data);
    std::vector<uint64_t> doutrec = thing.read(0x40600000,20);
    for (int i = 0; i < doutrec.size(); i++) {
        std::cout << std::hex << doutrec[i] << std::endl;
    }
    thing.closes(); 
    return 0;
}