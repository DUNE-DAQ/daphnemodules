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


void DaphneInterface::close() {

  ::close( m_connection_id );
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
  

std::vector<uint64_t>  DaphneInterface::read(uint8_t command_id,
					     uint64_t addr, uint8_t size) const {

  uint8_t cmd[10];
  cmd[0] = command_id;
  cmd[1] = size;
  memcpy(cmd + 2, &addr, sizeof(uint64_t));
  auto result = sendto(m_connection_id, cmd, sizeof(cmd), 0, (struct sockaddr*)&m_target, sizeof(m_target));

  if ( result < 0 ) throw FailedSocketInteraction(ERS_HERE, "sendto") ;
  
  uint8_t buffer[2 + (8 * size)];
  socklen_t addrlen = sizeof(m_target);
  result = recvfrom(m_connection_id, buffer, sizeof(buffer), 0, (struct sockaddr*)&m_target, &addrlen);

  if ( result <= 0 ) throw FailedSocketInteraction(ERS_HERE, "recvfrom") ;
  
  uint8_t fmt[4 + size];
  fmt[0] = '<';
  fmt[1] = 'B';
  fmt[2] = 'B';
  fmt[3] = size;
  for (int i = 0; i < size; i++) {
    fmt[4 + i] = 'Q';
  }

  std::vector<uint64_t> ret_value;
  for (int i = 0; i < size; i++) {
    uint64_t value;
    memcpy(&value, buffer + 2 + (8 * i), sizeof(uint64_t));
    ret_value.push_back(value);
  }

  return ret_value;
}


void  DaphneInterface::write(uint8_t command_id, uint64_t addr, std::vector<uint64_t> && data)  const {

  uint8_t cmd[10 + (8 * data.size())];
  cmd[0] = command_id;
  cmd[1] = data.size();
  memcpy(cmd + 2, &addr, sizeof(uint64_t));
  for (int i = 0; i < data.size(); i++) {
    memcpy(cmd + 10 + (8 * i), &(data[i]), sizeof(uint64_t));
  }

  auto result = sendto(m_connection_id, cmd, sizeof(cmd), 0, (struct sockaddr*)&m_target, sizeof(m_target));
  if ( result < 0 ) throw FailedSocketInteraction(ERS_HERE, "sendto") ;
}





