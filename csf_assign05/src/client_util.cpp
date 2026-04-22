/*
 * Cpp implementation for client util funcs
 * CSF Assignment 5 
 * Su Huang 
 * shuan148@jh.edu
 */

 #include "client_util.h"

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

bool login(int fd, ClientMode client_mode) {
  // get credentials 
  std::string username, password; 
  std::cout << "username: "; 
  std::getline(std::cin, username); 
  std::cout << "password: "; 
  std::getline(std::cin, password); 
  std::string credentials = username + '/' + password; 

  // send and receive login message
  Message login_send_msg(MessageType::LOGIN, client_mode, credentials); 
  Message login_recv_msg = send_recv(fd, login_send_msg); 

  // validate received message
  if (login_recv_msg.get_type() == MessageType::ERROR) {
    std::cerr << "Error: " << login_recv_msg.get_str() << "\n"; 
    return false; 
  } else if (login_recv_msg.get_type() != MessageType::OK) {
    std::cerr << "Error: unknown error from login" << "\n"; 
    return false; 
  }
  return true; 
} 
