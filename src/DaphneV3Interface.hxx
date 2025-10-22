/** 
 *  
 * Implementations of DaphneV3Interface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

#include "DaphneV3Interface.hpp"
#include "logging/Logging.hpp"

#include <fmt/format.h>
#include <regex>

#warning CHECK IF WE NEED THIS
#include <sys/time.h>

using namespace dunedaq::daphnemodules;
using namespace daphne;


template<class T>
T DaphneV3Interface::send( std::string && message, daphne::MessageTypeV2 sent_type, daphne::MessageTypeV2 received_type ) {

  std::unique_lock<std::mutex> lock(m_access_mutex);

  _send(std::move(message), sent_type);

  auto ret = _receive();

  lock.unlock();

  const auto ty = ret.type();
  T out;
  if ( ty != received_type ) {
    throw FailedDecoding(ERS_HERE, out.GetTypeName(), ret.payload(),
			 TypeMismatch(ERS_HERE, MessageTypeV2_Name(ty), MessageTypeV2_Name(received_type)) );
  }

  if (!out.ParseFromString(ret.payload())) {
    throw FailedDecoding(ERS_HERE, out.GetTypeName(), ret.payload());
  }

  return out;
}





