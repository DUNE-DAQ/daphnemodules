/**
 * @file DaphneInterface.hpp
 *
 * Definition of the interface protocol to the daphne boards
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 *
 */

#ifndef DAPHNEMODULES_SRC_DAPHNEINTERFACE_HPP_
#define DAPHNEMODULES_SRC_DAPHNEINTERFACE_HPP_ 

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <cstring>
#include <unistd.h>

#include <ers/ers.hpp>

namespace dunedaq {

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedPing,
		     "Failed to ping daphne board at " << ip << ':' << port,
		     ((std::string)ip)((int)port)
		   ) 
		    
  
} // dunedaq namespace


namespace dunedaq::daphnemodules {

  class DaphneInterface {

  public:
    DaphneInterface( const char* ipaddr, int port );
    ~DaphneInterface() {close();}

    std::vector<uint64_t> read_register(uint64_t addr, uint8_t size) const;
    void write_register(uint64_t addr, std::vector<uint64_t> && data)  const;

    std::vector<uint64_t> read_buffer(uint64_t addr, uint8_t size) const;
    void write_buffer(uint64_t addr, std::vector<uint64_t> && data) const;

    void close();
    bool ping() const noexcept;
    
    
  private:
    int sock;
    struct sockaddr_in target;
  }; 
  

} // namespce  dunedaq::daphnemodules


#endif // DAPHNEMODULES_SRC_DAPHNEINTERFACE_HPP_
