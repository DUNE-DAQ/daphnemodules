/** 
 *  
 * Implementations of DaphneV3Interface's functions                                                                    
 * 
 * This is part of the DUNE DAQ Software Suite, copyright 2020.                                                      
 * Licensing/copyright details are in the COPYING file that you should have         
 */

 
template<class T>
T dunedaq::daphnemodules::DaphneV3Interface::send( std::string && message, daphne::MessageTypeV2 sent_type, daphne::MessageTypeV2 received_type ) {

  std::unique_lock<std::mutex> lock(m_access_mutex);

  const uint64_t msg_id = m_message_counter++;
  _send(std::move(message), sent_type, msg_id);

  ControlEnvelopeV2 ret;
  while (true) {
    ret = _receive();
    if (ret.correl_id() == msg_id && ret.type() == received_type) break;
    ers::warning(TypeMismatch(ERS_HERE,
                              MessageTypeV2_Name(ret.type()),
                              MessageTypeV2_Name(received_type)));
  }

  lock.unlock();

  T out;
  if (!out.ParseFromString(ret.payload())) {
    throw FailedDecoding(ERS_HERE, out.GetTypeName(), ret.payload());
  }

  return out;
}



