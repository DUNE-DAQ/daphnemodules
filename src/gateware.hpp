#include <stdio.h>
#include "oei.hpp"
#include <iostream>
#include <string>
using namespace std;

class config {
public:
    config(const char* ip_address, int ep) {
        OEI thing(ip_address);
        char c[1000];
        FILE *fptr;
        if ((fptr = fopen("gateware.d", "r")) == NULL) {
            printf("Error! gateware.d does exist?.");
            exit(1);
        }

        fscanf(fptr, "%[^\n]", c);
        printf("Data from the file:\n%s  ", c);
        fclose(fptr);
        
        int gateware = (thing.read(0x9000, 1)[0]);
        //printf("%d\n",gateware);
        //printf("%d\n",stoi(c));
        if ( gateware == stoi(c)) {
            std::cout << "success" << std::endl;
        } else {
            std::cout << "Warning! DAPHNE is programmed with old gateware!\n" << std::endl;
        }
        thing.closes();
    }
};
