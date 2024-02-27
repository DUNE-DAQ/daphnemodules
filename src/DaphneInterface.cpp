/** 
 *  
 * Implementations of DaphneInterface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

#include "DaphneInterface.hpp"

using namespace dunedaq::daphnemodules;

DaphneInterface::DaphneInterface( const char* ipaddr, int port ) {
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  target.sin_family = AF_INET;
  target.sin_port = htons(port);
  inet_pton(AF_INET, ipaddr, &(target.sin_addr));

  // if the ping fails, we should throw an ERS
  if ( ! ping() )
    throw FailedPing( ERS_HERE, ipaddr, port )

}


std::vector<uint64_t>  DaphneInterface::read_register(uint64_t addr, uint8_t size) const {

  

}


void  DaphneInterface::write_register(uint64_t addr, std::vector<uint64_t> && data)  const {

}


std::vector<uint64_t> DaphneInterface::read_buffer(uint64_t addr, uint8_t size) const {

}

void DaphneInterface::write_buffer(uint64_t addr, std::vector<uint64_t> && data) const {

}


void DaphneInterface::close() {

}


bool DaphneInterface::ping() const noexcept {

  // MR: We should ping the board here. And not with a system call
  // Something like this should work.

  // Reference:  https://bbs.archlinux.org/viewtopic.php?id=213878
 
  struct timeval timeout;
  timeout.tv_sec = 1 ;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
  
  if (connect(sock, (struct sockaddr *) &addr, sizeof(addr)) != 0)
    return true;
  else
    return false;

}
  





