#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <logging/Logging.hpp>


namespace dunedaq::daphnemodules {
class testip {
public:
	testip(const char* ip) {
    //using namespace std;
    std::string command(std::string("ping -c1 -s1 ") + ip);
    int x = system((command + "> /dev/null 2>&1").c_str());
    if (x==0){
    TLOG()<<"success\n";
    }else{
    TLOG()<< "ip address unreacheble\n";
    }
    }
};
}
