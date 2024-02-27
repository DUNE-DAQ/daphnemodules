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
  
  m_connection_id = socket(AF_INET, SOCK_DGRAM, 0);

  if ( m_connection_id < 0 )
    throw SocketCreationError(ERS_HERE);
  
  m_target.sin_family = AF_INET;
  m_target.sin_port = htons(port);
  auto ret = inet_pton(AF_INET, ipaddr, &(m_target.sin_addr));
  if ( ret <= 0 ) 
    throw InvalidIPAddress(ERS_HERE, ipaddr);
 
  if ( ! ping() )
    throw FailedPing(ERS_HERE, ipaddr, port );

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


bool DaphneInterface::ping(int timeout_s, int timeout_usec) const noexcept {

  // MR: We should ping the board here. And not with a system call
  // Something like this should work.

  // Reference:  https://bbs.archlinux.org/viewtopic.php?id=213878
 
  struct timeval timeout;
  timeout.tv_sec  = timeout_s ;
  timeout.tv_usec = timeout_usec ;
  setsockopt(m_connection_id, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
  setsockopt(m_connection_id, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
  
  if (connect(m_connection_id, (struct sockaddr *) & m_target, sizeof(m_target)) != 0)
    return true;
  else
    return false;

}
  





