#include <iostream>
#include <string>
#include <cassert>
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "server.h"
#include "passwd_db.h"
#include "client.h"

Client::Client(int fd, Server *server)
  : m_fd(fd)
  , m_server(server) {
  // TODO: do any necessary initialization
}

Client::~Client() {
  // TODO: do any necessary cleanup
}

void Client::chat() {
  // TODO: implement
  try {
    handle_login(); 
  } catch (const IOException &e) {
    return; 
  } catch (const ProtocolError &e) {
    return; 
  } catch (const SemanticError &e) {
    try {
      send_msg(Message(MessageType::ERROR, e.what())); 
    } catch (...) {}
  }
  return; 
}

// TODO: implement other member functions
void Client::send_msg(const Message &msg) {
  std::string encoded_str; 
  Wire::encode(msg, encoded_str); 
  IO::send(encoded_str, m_fd); 
}

Message Client::recv_msg() {
  Message response_msg; 
  std::string response_str; 
  IO::receive(m_fd, response_str); 
  Wire::decode(response_str, response_msg); 
  return response_msg; 
}

void Client::handle_login() {
  // get login messagetype
  Message login_msg = recv_msg(); 
  if (login_msg.get_type() != MessageType::LOGIN) {
    throw ProtocolError("expected login messagetype"); 
  }

  // get username/password from msg string
  std::string credentials = login_msg.get_str(); 
  size_t pos = credentials.find('/'); 
  if (pos == std::string::npos) {
    throw SemanticError("invalid credentials format"); 
  }

  // validate username and password 
  if (!PasswordDB::authenticate(credentials)) {
    throw SemanticError("invalid username/password"); 
  }

  // set client mode 
  m_mode = login_msg.get_client_mode(); 
  if (m_mode == ClientMode::INVALID) {
    throw SemanticError("invalid client mode"); 
  }

  // send ok messagetype
  send_msg(Message(MessageType::OK, "successful login")); 
}

