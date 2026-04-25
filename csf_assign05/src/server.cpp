/*
 * Cpp implementation for server
 * CSF Assignment 5 
 * Su Huang 
 * shuan148@jh.edu
 */

 #include <iostream>
#include <memory>
#include <cstring> // for strerror()
#include <cerrno>  // for errno
#include "csapp.h"
#include "except.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "client.h"
#include "guard.h"
#include "server.h"

namespace {

// worker thread detaches itself and calls chat 
void *worker(void *arg) {
  pthread_detach(pthread_self());

  std::unique_ptr<Client> client(static_cast<Client*>(arg));
  
  try {
    client->chat(); 
  } catch (const std::exception &e) {
    std::cerr << "client chat exception: " << e.what() << "\n"; 
  } catch (...) {
    std::cerr << "non-standard client chat exception" << "\n"; 
  }

  return NULL; 
}

// accepts a client connection 
int accept_connection(int ssock_fd, struct sockaddr_in client_addr) {
  unsigned client_len = sizeof(client_addr); 
  int child_fd = accept(ssock_fd, (struct sockaddr *) &client_addr, &client_len); 

  // return -1 to indicate accept failed 
  if (child_fd < 0) {
    std::cerr << "accept failed: " << std::strerror(errno) << "\n"; 
    return -1; 
  }

  return child_fd; 
}

}

Server::Server() {
  pthread_mutex_init(&m_mutex, NULL); 
  m_next_order_id = 1000;                  // actual order ids begin at 1000 
}

Server::~Server() {
  pthread_mutex_destroy(&m_mutex); 
}

void Server::server_loop(const char *port) {
  int server_fd = open_listenfd(port);
  if (server_fd < 0)
    throw IOException(std::string("open_listenfd failed: ") + std::strerror(errno));

  while (true) {
    struct sockaddr_in client_addr; 
    int client_fd = accept_connection(server_fd, client_addr); 

    // try to establish connection again if accept failed 
    if (client_fd < 0) {
      continue; 
    }

    // instance of client class to manage resources of client thrread 
    Client *client = new Client(client_fd, this); 

    pthread_t thr_id; 
    if (pthread_create(&thr_id, NULL, worker, client)!= 0) {
      std::cerr << "pthread_create failed\n"; 
      delete client; 
    }
  }
}

std::shared_ptr<Order> Server::order_new(std::shared_ptr<Order> order) {
  // modifying shared server so need critical section 
  Guard g(m_mutex); 

  // process new order 
  int order_id = m_next_order_id++; 
  order->set_id(order_id); 
  order->set_status(OrderStatus::NEW); 

  // update each item's order id
  for (int i = 0; i < order->get_num_items(); i++) {
    order->at(i)->set_order_id(order_id); 
  }
  m_orders_map[order_id] = order; 
  
  // broadcast message to display clients 
  auto msg = std::make_shared<Message>(MessageType::DISP_ORDER, order); 
  broadcast(msg); 

  return order; 
} 

void Server::order_update(int order_id, OrderStatus new_order_status) {
  // modifying shared server so need critical section 
  Guard g(m_mutex); 

  // validate and assign order transition 
  auto order = valid_order_id(order_id); 
  if (order == NULL) {
    throw SemanticError("order not found"); 
  }
  if (!valid_next_order_status(order->get_status(), new_order_status)) {
    throw SemanticError("invalid order status update"); 
  }
  order->set_status(new_order_status); 

  // broadcast message to display clients 
  auto msg = std::make_shared<Message>(MessageType::DISP_ORDER_UPDATE, order_id, new_order_status); 
  broadcast(msg); 

  // delete order if now delivered 
  if (new_order_status == OrderStatus::DELIVERED) {
    m_orders_map.erase(order_id); 
  }
}

void Server::item_update(int order_id, int item_id, ItemStatus new_item_status) {
  // modifying shared server so need critical section 
  Guard g(m_mutex); 

  // validate and assign item transition 
  auto order = valid_order_id(order_id); 
  if (!order) {
    throw SemanticError("order not found"); 
  }
  auto item = order->find_item(item_id); 
  if (!item) {
    throw SemanticError("item not found"); 
  }
  if (!valid_next_item_status(item->get_status(), new_item_status)) {
    throw SemanticError("invalid item status update"); 
  }
  item->set_status(new_item_status); 

  // broadcast message to display clients 
  auto msg = std::make_shared<Message>(MessageType::DISP_ITEM_UPDATE, order_id, item_id, new_item_status); 
  broadcast(msg); 

  // set order to in progress if it's the first item set to in progress
  if (first_in_progress(order, item_id)) {
    order_update_internal(order, OrderStatus::IN_PROGRESS); 
  }

  // set order to done if all items are now done 
  if (all_items_done(order)) {
    order_update_internal(order, OrderStatus::DONE); 
  }
}

void Server::add_display_client(Client *client) {
  // modifying shared server so need critical section 
  Guard g(m_mutex); 

  // add client to set 
  std::cerr << "Adding client address: " << client << std::endl;
  m_display_clients_set.insert(client); 
}

void Server::remove_display_client(Client *client) {
  // modifying shared server so need critical section 
  Guard g(m_mutex); 

  // remove client from set 
  m_display_clients_set.erase(client); 
}

std::vector<std::shared_ptr<Order>> Server::get_all_orders() {
  // acessing shared server so need critical section 
  Guard g(m_mutex); 

  // iterate through map and save orders in vector 
  std::vector<std::shared_ptr<Order>> orders_vec; 
  for (auto &orderid_order : m_orders_map) {
    orders_vec.push_back(orderid_order.second); 
  }

  return orders_vec; 
}

std::shared_ptr<Order> Server::valid_order_id(int order_id) const {
  // verify that order id exists 
  auto i = m_orders_map.find(order_id); 
  if (i == m_orders_map.end()) {
    return std::shared_ptr<Order>(); 
  }
  return i->second; 
}

bool Server::valid_next_order_status(OrderStatus curr_status, OrderStatus next_status) const {
  // only allow the following sequences of order status changes 
  if (curr_status == OrderStatus::NEW) {
    return next_status == OrderStatus::IN_PROGRESS; 
  } else if (curr_status == OrderStatus::IN_PROGRESS) {
    return next_status == OrderStatus::DONE; 
  } else if (curr_status == OrderStatus::DONE) {
    return next_status == OrderStatus::DELIVERED; 
  } 

  return false; 
}

bool Server::valid_next_item_status(ItemStatus curr_status, ItemStatus next_status) const {
  // only allow the following sequences of item status changes 
  if (curr_status == ItemStatus::NEW) {
    return next_status == ItemStatus::IN_PROGRESS; 
  } else if (curr_status == ItemStatus::IN_PROGRESS) {
    return next_status == ItemStatus::DONE; 
  }

  return false; 
}

bool Server::first_in_progress(const std::shared_ptr<Order> order, int item_id) const {
  // early termination if given item isn't in progress 
  if (order->find_item(item_id)->get_status() != ItemStatus::IN_PROGRESS) {
    return false; 
  }

  // return false if any other items isn't new 
  for (int i = 0; i < order->get_num_items(); i++) {
    if (order->at(i)->get_status() != ItemStatus::NEW && order->at(i)->get_id() != item_id) {
      return false; 
    }
  }

  return true; 
}
  
bool Server::all_items_done(const std::shared_ptr<Order> order) const {
  // return false if any item isn't done 
  for (int i = 0; i < order->get_num_items(); i++) {
    if (order->at(i)->get_status() != ItemStatus::DONE) {
      return false; 
    }
  }

  return true; 
}

void Server::order_update_internal(const std::shared_ptr<Order> order, OrderStatus new_order_status) {
  // assign order new status 
  order->set_status(new_order_status); 

  // broadcast message to display clients 
  auto msg = std::make_shared<Message>(MessageType::DISP_ORDER_UPDATE, order->get_id(), new_order_status); 
  broadcast(msg); 

  // delete order if now delivered 
  if (new_order_status == OrderStatus::DELIVERED) {
    m_orders_map.erase(order->get_id()); 
  }
}

void Server::broadcast(const std::shared_ptr<Message> msg) const {
  for (Client *client : m_display_clients_set) {
    // enqueue a distinct copy of the message for each client 
    client->enqueue_display_msg(msg->duplicate()); 
  }
}

