/**

@mainpage DAPHNE-libs
@author Manuel Arroyave (marroyav)

@description This is a collection of libraries to monitor, config and spy daphne registers.
All the libraries are based in oei, a class with 4 functions to read and write registers via ETH.

Basic functionality would support endpoint status.
Endpoint_status.hpp seems like the library to test integration since it's only readout 
and returns feedback about the status of the timing endpoint.

Stand alone applications:
testip.cpp          -------> pings to every daphne board and prints success or fail
gateware.cpp        -------> reads the hash of the gateware running on each daphne and compares with a hash in a txt file
endpoint_status.cpp -------> prints clk and timing variables on terminal
clocks.cpp          -------> executes clk config for all daphne boards, (requires echo after)
ts.cpp              -------> prints 4 consecutive ts from each board, (those must be consecutive)
select_mode.cpp     -------> configures all daphnes in default data stream config.
read20.cpp          -------> prints 20 words from the spy buffer at the output of the 0 transceiver, (ip address is hardcoded)

*/
