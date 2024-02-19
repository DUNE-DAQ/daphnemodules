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
gateware.cpp        -------> takes ip address and ask for the gateware running on each daphne  
endpoint_status.cpp -------> prints clk and timing variables on terminal  
clocks.cpp          -------> execute clk config for all daphne boards. (requires echo after)  
ts.cpp              -------> Prints 4 consecutive ts from each board  
select_mode.cpp     -------> Configures all daphnes in default data stream config  
read20.cpp          -------> Prints 20 words from the spy buffer at the output of the 0 transceiver  

These are based in hpp files with functions to automatize sending commands for specific interfaces  

The Booting procedure for all DAPHNEs in the detector is as follows:

1. Power on
2. Check ping on IP Address ----> send warnings and set a vector only with pingable IP Addresses
3. Check firmware version   ----> warning if it's different among the set of running daphnes
4. Check timing registers   ----> set timing to work with timing interface
5. Send Echo/Alignment command from timing interface (calls a bash script by hand?)
6. Check endpoint           ----> send warnings if state is not good to go
6. Align the AFEs and check ----> send warnings if registers are not 0x3f80
7. Set analog chain         ----> this process takes some time (~few min)
8. Fine tunning the offset  ----> we can skip it but It might improve dynamic range in some channels

Calibration:

We might need several (tens of) runs for calibration  
Procedure is very easy from DAQ point of view:  
Is it possible to automatize this and scan using one variable of the calibration module?  

1. Configure calibration module
2. Configure DAPHNE
3. Lauch run
4. save metadata (configuration parameters)

*/

