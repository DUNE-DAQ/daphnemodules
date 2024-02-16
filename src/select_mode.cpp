#include "oei.hpp"

int main() {
    std::vector<int> full_stream_ep = {4, 5, 7};
    std::vector<int> self_triggr_ep = {9, 11, 12, 13};
    std::vector<int> disable;

    for (int i : full_stream_ep) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        OEI thing((ip).c_str());
        std::cout << "address= 10.73.137." << 100 + i << std::endl;
        std::cout << "parameters = " << std::hex << thing.read(0x3000, 1)[0] << std::endl;
        thing.write(0x3001, {0xaa});
        std::cout << "data mode = " << std::hex << thing.read(0x3001, 1)[0] << std::endl;
        thing.write(0x6001, {0b00000000});
        std::cout << "channels active = " << thing.read(0x6001, 1)[0] << std::endl;
        std::cout << "reg 0x5007 = " << thing.read(0x5007, 1)[0] << std::endl;
        thing.closes();
    }

    for (int i : self_triggr_ep) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        OEI thing((ip).c_str());
        std::cout << "address= 10.73.137." << 100 + i << std::endl;
        std::cout << "parameters = " << std::hex << thing.read(0x3000, 1)[0] << std::endl;
        thing.write(0x3001, {0x3});
        std::cout << "data mode = " << std::hex << thing.read(0x3001, 1)[0] << std::endl;
        thing.write(0x6000, {700});
        std::cout << "threshhold = " << thing.read(0x6000, 1)[0] << std::endl;
        thing.write(0x6001, {0b11111111});
        std::cout << "channels active = " << thing.read(0x6001, 1)[0] << std::endl;
        std::cout << "reg 0x5007 = " << thing.read(0x5007, 1)[0] << std::endl;
        thing.closes();
    }

    for (int i : disable) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        OEI thing((ip).c_str());
        std::cout << "address= 10.73.137." << 100 + i << std::endl;
        std::cout << "parameters = " << std::hex << thing.read(0x3000, 1)[0] << std::endl;
        thing.write(0x3001, {0x0});
        std::cout << "data mode = " << std::hex << thing.read(0x3001, 1)[0] << std::endl;
        thing.write(0x6001, {0b00000000});
        std::cout << "channels active = " << thing.read(0x6001, 1)[0] << std::endl;
        thing.closes();
    }
    return 0;
}
