/** 
 *  
 * Implementations of DaphneInterface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

#include "DaphneInterface.hpp"
#include "logging/Logging.hpp"
#include <sys/time.h>

using namespace dunedaq::daphnemodules;

DaphneInterface::DaphneInterface( const char* ipaddr, int port, std::chrono::microseconds timeout )
  : m_socket_timeout(timeout) {

  m_connection_id = socket(AF_INET, SOCK_DGRAM, 0);
  
  if ( m_connection_id < 0 )
    throw SocketCreationError(ERS_HERE);
  
  m_target.sin_family = AF_INET;
  m_target.sin_port = htons(port);
  auto ret = inet_pton(AF_INET, ipaddr, &(m_target.sin_addr));
  if ( ret <= 0 ) 
    throw InvalidIPAddress(ERS_HERE, ipaddr);

 
  if ( ! validate_connection() )
    throw FailedPing(ERS_HERE, ipaddr, port );

}


void DaphneInterface::close() {

  ::close( m_connection_id );
}


bool DaphneInterface::validate_connection() const {

  auto ret = read_register( 0xaa55, 1);

  static const uint64_t good_value = 0xdeadbeef; 
  return ret[0] == good_value ;

}


command_result DaphneInterface::send_command( std::string cmd ) const {

  bool sent = false;
  do {
    try {
      auto ret = send_command(cmd, std::chrono::milliseconds(50) );
      sent = true;
      return ret;
    }
    catch ( const CommandTimeout & e ) {
      ers::warning( e );
    }
    catch ( const ers::Issue & e ) {
      throw FailedSocketInteraction(ERS_HERE, cmd, e);
    }
			   
  } while (! sent);

  
}



command_result DaphneInterface::send_command( std::string cmd, std::chrono::milliseconds timeout ) const {

  TLOG() << "Sending command " << cmd;
  std::vector<uint64_t> bytes;
  for (char ch : cmd) {
    bytes.push_back(static_cast<uint64_t>(ch));
  }
  bytes.push_back(0x0d); // dedicated command flag

  const std::lock_guard<std::mutex> lock(m_command_mutex);
  
  // we send the bytes in chunks of 50 words
  for (size_t i = 0; i < (bytes.size() + 49) / 50; ++i) {
    std::vector<uint64_t> part(bytes.begin() + i*50,
			       bytes.begin() + std::min((i+1)*50, bytes.size()));
    write_buffer(0x90000000, std::move(part));
  }

  TLOG() << "Command sent, waiting for result";

  
  command_result res;
  std::string * writing_pointer = nullptr;

  auto start_time = std::chrono::high_resolution_clock::now();
  
  int more = 40;
  while (more > 0) {
    auto data_block = read_buffer(0x90000000, 50);
    for (size_t i = 0; i < data_block.size(); ++i) {
      if (data_block[i] == 255) {
	break;
      } else if (data_block[i] == 1) {
	// the following data are returning the command that was issued
	writing_pointer = & res.command;
      } else if (data_block[i] == 2) {
	// the following data are the immediate command response
	writing_pointer = & res.result;
      } else if (data_block[i] == 3) {
	// this is the message end
	writing_pointer = nullptr;
      } else if (isprint(static_cast<int>(data_block[i]))) {
	more = 40;
	char c = static_cast<char>(data_block[i]);
	if ( writing_pointer ) {
	  *writing_pointer += static_cast<char>(data_block[i]);
	}
	else {
	  TLOG() << "Failed adding charachter " <<  c;
	}
      }
    }
    auto now = std::chrono::high_resolution_clock::now();

    auto delay = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

    if ( delay > timeout ) {
      TLOG() << "Details of timeout";
      for ( size_t i = 0; i < data_block.size(); ++i ) {
	TLOG() << i << "\t" << std::hex << data_block[i] << std::dec;
      }
      TLOG() << "Received so far: " << res.result;
      throw CommandTimeout(ERS_HERE, cmd, delay.count());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    --more;
  }

  return res;
}


std::vector<uint64_t>  DaphneInterface::read(uint8_t command_id,
					     uint64_t addr, uint8_t size) const {

  const std::lock_guard<std::mutex> lock(m_access_mutex);

  uint8_t cmd[10];
  cmd[0] = command_id;
  cmd[1] = size;
  memcpy(cmd + 2, &addr, sizeof(uint64_t));
  auto result = sendto(m_connection_id, cmd, sizeof(cmd), 0, (struct sockaddr*)&m_target, sizeof(m_target));

  if ( result < 0 ) throw FailedSocketInteraction(ERS_HERE, "sendto") ;


  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = m_socket_timeout.count();
  fd_set readfds, masterfds;

  FD_ZERO(&masterfds);
  FD_SET(m_connection_id, &masterfds);
  
  memcpy(&readfds, &masterfds, sizeof(fd_set));

  auto start_time = std::chrono::high_resolution_clock::now();
  
  if (select(m_connection_id+1, &readfds, NULL, NULL, &timeout) < 0) {
    throw FailedSocketInteraction(ERS_HERE, "select") ;
  }

  std::vector<uint64_t> ret_value;
  
  if (FD_ISSET(m_connection_id, &readfds)) {
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

    for (int i = 0; i < size; i++) {
      uint64_t value;
      memcpy(&value, buffer + 2 + (8 * i), sizeof(uint64_t));
      ret_value.push_back(value);
    }

  }
  else {
    // the socket timedout
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    throw SocketTimeout(ERS_HERE, duration.count() );
  }
  
  return ret_value;
}


void  DaphneInterface::write(uint8_t command_id, uint64_t addr, std::vector<uint64_t> && data)  const {

  const std::lock_guard<std::mutex> lock(m_access_mutex);
  
  uint8_t cmd[10 + (8 * data.size())];
  cmd[0] = command_id;
  cmd[1] = data.size();
  memcpy(cmd + 2, &addr, sizeof(uint64_t));
  for (size_t i = 0; i < data.size(); i++) {
    memcpy(cmd + 10 + (8 * i), &(data[i]), sizeof(uint64_t));
  }

  auto result = sendto(m_connection_id, cmd, sizeof(cmd), 0, (struct sockaddr*)&m_target, sizeof(m_target));
  if ( result < 0 ) throw FailedSocketInteraction(ERS_HERE, "sendto") ;
}





