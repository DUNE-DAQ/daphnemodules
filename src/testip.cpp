#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>

class config {
public:
    config(const char* ip) {
        //using namespace std;
        std::string command(std::string("ping -c1 -s1 ") + ip);
        int x = system((command + "> /dev/null 2>&1").c_str());
        if (x==0){
        std::cout<<"success\n";
        }else{
        std::cout<<"failed\n";
        }
    }
};

int main() {
    for (int i : {4, 5, 7, 9, 11, 12, 13}) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        config c(ip.c_str());
    }
    return 0;
}