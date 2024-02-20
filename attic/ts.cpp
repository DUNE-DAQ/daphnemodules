#include "oei.hpp"

int main() {
    std::vector<int> nums = {4, 5, 7, 9, 11, 12, 13};
    
    for (int i : nums) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        OEI thing((ip).c_str());
        thing.write(0x2000, {1234});
        std::cout << "time stamp in DAPHNE with ip address " << "10.73.137." << 100 + i << std::endl;
        std::vector<uint64_t> a = thing.read(0x40500000, 4);
        for (int r = 0; r < a.size(); r++) {
            std::cout << a[r ] << std::endl;
        }
        thing.closes();
    }
    return 0;
}