#include "oei.hpp"

int main() {
    std::cout << "DAPHNE data scheme" << std::endl;
    std::cout << "ADDRESS    " << "\t";
    std::cout << "SLOT" << "\t";
    std::cout << "REG" << "\t";
    std::cout << "MODE" << std::endl;
    
    for (int i : {4, 5, 7, 9, 11, 12, 13}) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        OEI thing((ip).c_str());
        std::cout << ip << "\t";
        
        int slot = (thing.read(0x3000, 1)[0] >> 22);
        int sender = thing.read(0x3001, 1)[0];
        
        std::cout << slot << "\t";
        std::cout << std::hex << sender << "\t";
        
        if (sender == 0xaa) {
            std::cout << "full streaming" << std::endl;
        } else if (sender == 0x3) {
            std::cout << "self trigger" << std::endl;
        } else {
            std::cout << "disabled" << std::endl;
        }
        
        thing.closes();
    }
    
    return 0;
}
