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

  ERS_DECLARE_ISSUE( daphnemodules,
		     FailedSocketInteraction,
		     "Failed to call " << command,
		     ((std::string)command)
		   ) 
  
  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidBiasCtrlConfiguration,
		     "Invalid BiasCtrl Configuration " << v_biasctrl,
		     ((std::string)v_biasctrl)
		   ) 

  ERS_DECLARE_ISSUE( daphnemodules,
		     InvalidAFEConfiguration,
		     "Invalid AFE Configuration AFE Id" << afe.id << 'Register 52' << afe.conf.reg_52 <<  
		     'Register 4' << afe.conf.reg_4 << 'Register 52' << afe.conf.reg_52 << 
		     'Offset Gain' << afe.conf.v_gain << 'BIAS' << afe.conf.v_bias,
		     ((std::string)afe.id)((std::string)afe.conf.reg_52)((std::string)afe.conf.reg_4)((std::string)afe.conf.reg_52)
                     ((std::string)afe.conf.v_gain)((std::string)afe.conf.v_bias) 
		   )
  } // dunedaq namespace


namespace dunedaq::daphnemodules {

  struct command_result{
    std::string command;
    std::string result;
  };
  
  
  class DaphneInterface {

  public:
    DaphneInterface( const char* ipaddr, int port );
    ~DaphneInterface() { if(m_connection_id>0) close();}

    DaphneInterface(const DaphneInterface &) = delete;
    DaphneInterface & operator= (const DaphneInterface & ) = delete;
    DaphneInterface(DaphneInterface &&) = delete;
    DaphneInterface & operator= (DaphneInterface &&) = delete;
    
    std::vector<uint64_t> read_register(uint64_t addr, uint8_t size) const { return read(0x00, addr, size) ; }
    void write_register(uint64_t addr, std::vector<uint64_t> && data)  const { write(0x01, addr, std::move(data)) ; }

    std::vector<uint64_t> read_buffer(uint64_t addr, uint8_t size) const { return read(0x08, addr, size) ; }
    void write_buffer(uint64_t addr, std::vector<uint64_t> && data) const { write(0x09, addr, std::move(data)) ; }
 
    bool validate_connection() const ;

    command_result send_command( std::string cmd ) const ;
    
  protected:
    void close();

    void write( uint8_t command_id, uint64_t addr, std::vector<uint64_t> && data) const;
    std::vector<uint64_t> read(uint8_t command_id, uint64_t addr, uint8_t size) const;

    
  private:
    int m_connection_id = -1;
    sockaddr_in m_target;
  }; 
  

} // namespce  dunedaq::daphnemodules


#endif // DAPHNEMODULES_SRC_DAPHNEINTERFACE_HPP_
