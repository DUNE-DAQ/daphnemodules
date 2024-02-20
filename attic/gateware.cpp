#include "gateware.hpp"

/* This is a stand alone application to test the gateware version of daphne
 * comparing with the last version loaded on the boards which is updated
 * manually to the file <gateware.d>
 * 
 * The aproppiate behaviour of the application relies on the eth connection
 * for this reason testip.cpp should run succesfully before runnin the 
 * present application.
 * We use gateware.hpp who takes the ip and ask for the firmware running on 
 * the board.
 */


int main() {
    for (int i : {4, 5, 7, 9, 11, 12, 13}) {
        std::string ip = ("10.73.137." + std::to_string(i + 100));
        config c((ip).c_str(),i);
    }
    return 0;
}
