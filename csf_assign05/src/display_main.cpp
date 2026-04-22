// Main program for the display client

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

namespace {

// This terminal escape sequence should be written to
// std::cout before the display client redisplays the current
// order information.
const char CLEAR_SCREEN[] = "\x1b[2J\x1b[H";

}

// helper function to print orders to cout 
void display_orders(const std::map<int, std::shared_ptr<Order>> &orders_map) {
  // flush screen 
  std::cout << CLEAR_SCREEN << std::flush;

  // iterate through each order 
  for (const auto &key_val : orders_map) {
    const auto &order = key_val.second; 
    std::cout << "Order " << order->get_id() << ": " << Wire::order_status_to_str(order->get_status()) << "\n"; 
    
    // iterate through each item
    for (int i = 0; i < order->get_num_items(); i++) {
      auto item = order->at(i);  
      std::cout << "  Item " << item->get_id() << ": " << Wire::item_status_to_str(item->get_status()) << "\n"; 
      std::cout << "    " << item->get_desc() << ", Quantity " << item->get_qty() << "\n"; 
    }
  }
}

// helper function to add new order 
void disp_order(const Message &msg, std::map<int, std::shared_ptr<Order>> &orders_map) {
  // DISP_ORDER data values: order 
  auto order = msg.get_order(); 

  // add order 
  orders_map[order->get_id()] = msg.get_order(); 

  display_orders(orders_map); 
}

// helper function to update item status 
void disp_item_update(const Message &msg, std::map<int, std::shared_ptr<Order>> &orders_map) {
  // DISP_ITEM_UPDATE data values: order id, item id, item status
  auto order = orders_map[msg.get_order_id()]; 
  auto item = order->find_item(msg.get_item_id()); 

  // update status 
  item->set_status(msg.get_item_status()); 

  display_orders(orders_map); 
}

// helper function to update order status 
void disp_order_update(const Message &msg, std::map<int, std::shared_ptr<Order>> &orders_map) {
  // DISP_ORDER_UPDATE data values: order id, order status
  // delete order if delivered 
  if (msg.get_order_status() == OrderStatus::DELIVERED) {
    orders_map.erase(msg.get_order_id()); 
  } else {
    auto order = orders_map[msg.get_order_id()]; 

    // update status
    order->set_status(msg.get_order_status());
  }
 
  display_orders(orders_map); 
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <hostname> <port>\n";
    return 1;
  }

  // connect to server 
  int fd = open_clientfd(argv[1], argv[2]); 
  
  // prompt user login 
  if (!login(fd, ClientMode::DISPLAY)) {
    close(fd); 
    return 1; 
  }

  // flush screen 
  std::cout << CLEAR_SCREEN << std::flush;

  // initialize map of orders 
  std::map<int, std::shared_ptr<Order>> orders_map;

  while (true) {

    // get recv message 
    Message msg; 
    std::string str; 
    IO::receive(fd, str); 
    Wire::decode(str, msg); 

    // determine command to perform 
    if (msg.get_type() == MessageType::DISP_ORDER) {
      disp_order(msg, orders_map); 
    } else if (msg.get_type() == MessageType::DISP_ITEM_UPDATE) {
      disp_item_update(msg, orders_map); 
    } else if (msg.get_type() == MessageType::DISP_ORDER_UPDATE) {
      disp_order_update(msg, orders_map); 
    } else if (msg.get_type() == MessageType::DISP_HEARTBEAT) {
    }
  }
  close(fd); 
  return 0; 
}
