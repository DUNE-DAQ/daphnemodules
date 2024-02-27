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
#include <memory>

#include <ers/ers.hpp>

namespace dunedaq {

  ERS_DECLARE_ISSUE( daphnemodules,
		     SocketCreationError,
		     "Failed to create a socket",
		     ERS_EMPTY
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidIPAddress,
		     "Invalid address: " << ip,
		     ((std::string)ip)
		   ) 

  
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
    ~DaphneInterface() { if(m_connection_id>0) close();}

    DaphneInterface(const DaphneInterface &) = delete;
    DaphneInterface & operator= (const DaphneInterface & ) = delete;
    DaphneInterface(DaphneInterface &&) = delete;
    DaphneInterface & operator= (DaphneInterface &&) = delete;
    
    std::vector<uint64_t> read_register(uint64_t addr, uint8_t size) const;
    void write_register(uint64_t addr, std::vector<uint64_t> && data)  const;

    std::vector<uint64_t> read_buffer(uint64_t addr, uint8_t size) const;
    void write_buffer(uint64_t addr, std::vector<uint64_t> && data) const;

    bool ping(int timeout_s = 1, int timeout_usec = 0) const noexcept;

  protected:
    void close();
    
  private:
    int m_connection_id = -1;
    sockaddr_in m_target;
  }; 
  

} // namespce  dunedaq::daphnemodules


#endif // DAPHNEMODULES_SRC_DAPHNEINTERFACE_HPP_
