#include "client_util.h"

// TODO: definitions of common clases/functions used by client programs
Message send_recv(int fd, const Message &msg) {
  // encode and send msg 
  std::string encoded_str; 
  Wire::encode(msg, encoded_str); 
  IO::send(encoded_str, fd); 

  // receive and decode msg 
  Message response_msg; 
  std::string response_str; 
  IO::receive(fd, response_str); 
  Wire::decode(response_str, response_msg); 
  
  return response_msg; 
}
