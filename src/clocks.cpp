#include "configclk.hpp"

int main() {
    for (int i : {4, 5, 7, 9, 11, 12, 13}) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        config c((ip).c_str(),i);
    }
    return 0;
}