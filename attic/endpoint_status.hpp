/** @file endpoint_status.hpp
 *  @brief Function request status of timing interface variables.
 *         requires oei.hpp
 *  @author Manuel Arroyave (marroyav)
 *  @bug There are potencial errors in the instanciation of the class.
 *       the ip addres provided has to be changed to char when giving the argument to the class.
 */

#include <chrono>
#include <thread>
#include "oei.hpp"

class config {
public:
    config(const char* ip_address, int ep) {
        OEI thing(ip_address);
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "--------------------------------------" << std::endl;
        std::cout << "DAPHNE ip address " << ip_address << std::endl;
        std::cout << "DAPHNE firmware version " << std::hex << thing.read(0x9000, 1)[0] << std::endl;
        std::cout << "test resgisters " << std::hex << thing.read(0xaa55, 1)[0] << std::endl;
        std::cout << "endpoint address " << std::hex << thing.read(0x4001, 1)[0] << std::endl;
        std::cout << "register 5001 " << std::hex << thing.read(0x5001, 1)[0] << std::endl;
        std::cout << "register 3000 " << std::hex << thing.read(0x3000, 1)[0] << std::endl;
        int epstat = thing.read(0x4000, 1)[0];
        if (epstat & 0x00000001) {
            std::cout << "MMCM0 is LOCKED OK" << std::endl;
        } else {
            std::cout << "Warning! MMCM0 is UNLOCKED, need a hard reset!" << std::endl;
        }
        if (epstat & 0x00000002) {
            std::cout << "Master clock MMCM1 is LOCKED OK" << std::endl;
        } else {
            std::cout << "Warning! Master clock MMCM1 is UNLOCKED!" << std::endl;
        }
        if (epstat & 0x00000010) {
            std::cout << "Warning! CDR chip loss of signal (LOS=1)" << std::endl;
        } else {
            std::cout << "CDR chip signal OK (LOS=0)" << std::endl;
        }
        if (epstat & 0x00000020) {
            std::cout << "Warning! CDR chip UNLOCKED (LOL=1)" << std::endl;
        } else {
            std::cout << "CDR chip LOCKED (LOL=0) OK" << std::endl;
        }
        if (epstat & 0x00000040) {
            std::cout << "Warning! Timing SFP module optical loss of signal (LOS=1)" << std::endl;
        } else {
            std::cout << "Timing SFP module optical signal OK (LOS=0)" << std::endl;
        }
        if (epstat & 0x00000080) {
            std::cout << "Warning! Timing SFP module NOT DETECTED!" << std::endl;
        } else {
            std::cout << "Timing SFP module is present OK" << std::endl;
        }
        if (epstat & 0x00001000) {
            std::cout << "Timing endpoint timestamp is valid" << std::endl;
        } else {
            std::cout << "Warning! Timing endpoint timestamp is NOT valid" << std::endl;
        }
        int ep_state = (epstat & 0xF00) >> 8;
        if (ep_state == 0) {
            std::cout << "Endpoint State = 0 : Starting state after reset" << std::endl;
        } else if (ep_state == 1) {
            std::cout << "Endpoint State = 1 : Waiting for SFP LOS to go low" << std::endl;
        } else if (ep_state == 2) {
            std::cout << "Endpoint State = 2 : Waiting for good frequency check" << std::endl;
        } else if (ep_state == 3) {
            std::cout << "Endpoint State = 3 : Waiting for phase adjustment to complete" << std::endl;
        } else if (ep_state == 4) {
            std::cout << "Endpoint State = 4 : Waiting for comma alignment, stable 62.5MHz phase" << std::endl;
        } else if (ep_state == 5) {
            std::cout << "Endpoint State = 5 : Waiting for 8b10 decoder good packet" << std::endl;
        } else if (ep_state == 6) {
            std::cout << "Endpoint State = 6 : Waiting for phase adjustment command" << std::endl;
        } else if (ep_state == 7) {
            std::cout << "Endpoint State = 7 : Waiting for time stamp initialization" << std::endl;
        } else if (ep_state == 8) {
            std::cout << "Endpoint State = 8 : Good to go!!!" << std::endl;
        } else if (ep_state == 12) {
            std::cout << "Endpoint State = 12 : Error in rx" << std::endl;
        } else if (ep_state == 13) {
            std::cout << "Endpoint State = 13 : Error in time stamp check" << std::endl;
        } else if (ep_state == 14) {
            std::cout << "Endpoint State = 14 : Physical layer error after lock" << std::endl;
        } else {
            std::cout << "Endpoint State = " << ep_state << " : warning! undefined state!" << std::endl;
        }
        thing.closes();
    }
};

