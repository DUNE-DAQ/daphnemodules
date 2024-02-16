#include <iostream>
using namespace std;
int main() {
    std::string command= ("ping -c1 -s1 10.73.137." +std::to_string (104) +"> /dev/null 2>&1" );
int x = system("ping -c1 -s1 10.73.137.104  > /dev/null 2>&1");
if (x==0){
    std::cout<<"success\n";
}else{
    std::cout<<"failed\n";
}}