#include <iostream>
#include <string>
#include <cassert>
#include <unistd.h>
#include <vector> 
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
  m_mode = ClientMode::INVALID; 
}

Client::~Client() {
  close(m_fd); 
}

void Client::chat() {
  try {
    handle_login(); 
    if (m_mode == ClientMode::UPDATER) {
      handle_updater_client(); 
    } else if (m_mode == ClientMode::DISPLAY) {
      handle_display_client(); 
    }
  } catch (const IOException &e) {
    return; 
  } catch (const ProtocolError &e) {
    return; 
  } catch (const SemanticError &e) {
    send_msg(Message(MessageType::ERROR, e.what())); 
  } 
  return; 
}

void Client::enqueue_display_msg(std::shared_ptr<Message> msg) {
  m_queue.enqueue(msg); 
}

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

void Client::handle_order_new(const Message &msg) {
  // verify order id
  if (msg.get_order()->get_id() != 1) {
    throw SemanticError("invalid order id"); 
  }

  // get server to create new order, respond with ok message 
  auto order = m_server->order_new(msg.get_order()); 
  send_msg(Message(MessageType::OK, "Created order id " + std::to_string(order->get_id()))); 
}

void Client::handle_item_update(const Message &msg) {
  try {
    // attempt to get server to update item, respond with ok message 
    m_server->item_update(msg.get_order_id(), msg.get_item_id(), msg.get_item_status()); 
    send_msg(Message(MessageType::OK, "updated item status")); 
  } catch (const SemanticError &e) {
    // catch error from server, respond with error message 
    send_msg(Message(MessageType::ERROR, e.what())); 
  } 
}

void Client::handle_order_update(const Message &msg) {
  try {
    // attempt to get server to update order, respond with ok message 
    m_server->order_update(msg.get_order_id(), msg.get_order_status()); 
    send_msg(Message(MessageType::OK, "updated order status")); 
  } catch (const SemanticError &e) {
    // catch error from server, respond with error message 
    send_msg(Message(MessageType::ERROR, e.what())); 
  }
}

void Client::handle_updater_client() {
  while (true) {
    Message msg = recv_msg(); 

    if (msg.get_type() == MessageType::QUIT) {
      // respond with ok message and exit loop 
      send_msg(Message(MessageType::OK, "exit")); 
      return; 
    } else if (msg.get_type() == MessageType::ORDER_NEW) {
      handle_order_new(msg); 
    } else if (msg.get_type() == MessageType::ITEM_UPDATE) {
      handle_item_update(msg); 
    } else if (msg.get_type() == MessageType::ORDER_UPDATE) {
      handle_order_update(msg); 
    }
  }
}

void Client::handle_display_client() {
  // send disp order messages for all orders 
  auto orders_vec = m_server->get_all_orders(); 
  for (auto &order : orders_vec) {
    send_msg(Message(MessageType::DISP_ORDER, order)); 
  }

  // ensure client gets broadcasts 
  m_server->add_display_client(this); 

  try {
    while (true) {
      auto msg = m_queue.dequeue(); 
      if (msg) {
        // send available message 
        send_msg(*msg); 
      } else {
        // no available message so send heartbeat 
        send_msg(Message(MessageType::DISP_HEARTBEAT, "")); 
      }
    }
  } catch (...) {
    // remove display client if exception 
    m_server->remove_display_client(this); 
    return; 
  }
}

