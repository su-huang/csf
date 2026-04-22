// Main program for the updater client

#include <iostream>
#include <string>
#include <stdexcept>
#include "csapp.h"
#include "model.h"
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"
#include "util.h"
#include "client_util.h"

// helper function to print message response success/failure 
void print_recv(const Message &msg) {
  if (msg.get_type() == MessageType::OK)
    std::cout << "Success: " << msg.get_str() << "\n";
  else
    std::cout << "Failure: " << msg.get_str() << "\n";
}

// helper function to parse a new order 
void order_new(int fd) {
  auto order = std::make_shared<Order>(1, OrderStatus::NEW); 

  // first line is num items 
  std::string num_items_str; 
  std::getline(std::cin, num_items_str); 
  int num_items = std::stoi(num_items_str); 

  // each item is entered as item id, item desc str, quantity 
  for (int i = 0; i < num_items; i++) {
    std::string item_id, item_desc, qty; 
    std::getline(std::cin, item_id); 
    std::getline(std::cin, item_desc); 
    std::getline(std::cin, qty); 

    // create item and add to order 
    auto new_item = std::make_shared<Item>(1, std::stoi(item_id), ItemStatus::NEW, item_desc, std::stoi(qty)); 
    order->add_item(new_item); 
  }

  // create, send, validate message with order 
  Message order_new_send_msg(MessageType::ORDER_NEW, order); 
  Message order_new_recv_msg = send_recv(fd, order_new_send_msg); 
  print_recv(order_new_recv_msg); 
}

// helper function to update an item 
void item_update(int fd) {
  // user enters order id, item id, item status 
  std::string order_id, item_id, item_status; 
  std::getline(std::cin, order_id); 
  std::getline(std::cin, item_id); 
  std::getline(std::cin, item_status); 

  // create, send, validate message with item update 
  Message item_update_send_msg(MessageType::ITEM_UPDATE, std::stoi(order_id), std::stoi(item_id), Wire::str_to_item_status(item_status)); 
  Message item_update_recv_msg = send_recv(fd, item_update_send_msg); 
  print_recv(item_update_recv_msg); 
}

// helper function to update an order 
void order_update(int fd) {
  // user enters order id, order status 
  std::string order_id, order_status; 
  std::getline(std::cin, order_id); 
  std::getline(std::cin, order_status); 

  // create, send, validate message with order update 
  Message order_update_send_msg(MessageType::ORDER_UPDATE, std::stoi(order_id), Wire::str_to_order_status(order_status)); 
  Message order_update_recv_msg = send_recv(fd, order_update_send_msg); 
  print_recv(order_update_recv_msg); 
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
    return 1;
  }

  // connect to server 
  int fd = open_clientfd(argv[1], argv[2]); 
  
  // wrap in try-catch to handle exceptions 
  try {
    // prompt user login 
    if (!login(fd, ClientMode::UPDATER)) {
      close(fd); 
      return 1; 
    }

    while (true) {
      // get user input 
      std::string user_input; 
      std::cout << "> "; 
      std::getline(std::cin, user_input); 

      // determine command to perform 
      if (user_input == "quit") {
        send_recv(fd, Message(MessageType::QUIT, std::string("quit"))); 
        break; 
      } else if (user_input == "order_new") {
        order_new(fd); 
      } else if (user_input == "item_update") {
        item_update(fd); 
      } else if (user_input == "order_update") {
        order_update(fd); 
      } 
    }
  } catch (const Exception &e) {
    std::cerr << "Error: " << e.what() << "\n"; 
    close(fd); 
    return 1; 
  }

  close(fd); 
  return 0; 

}
