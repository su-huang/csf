#include <string>
#include <unordered_map>
#include <cassert>
#include <memory> 
#include <vector> 
#include <iostream> 
#include <sstream> 
#include <algorithm>
#include <cctype> 
#include "util.h"
#include "model.h"
#include "except.h"
#include "wire.h"

namespace {

// Private data and helper functions

const std::unordered_map<ClientMode, std::string> s_client_mode_to_str = {
  { ClientMode::INVALID, "INVALID" },
  { ClientMode::UPDATER, "UPDATER" },
  { ClientMode::DISPLAY, "DISPLAY" },
};

const std::vector<std::pair<std::string, ClientMode>> s_str_to_client_mode_vec =
  Util::invert_unordered_map(s_client_mode_to_str);

const std::unordered_map<std::string, ClientMode> s_str_to_client_mode(
  s_str_to_client_mode_vec.begin(),
  s_str_to_client_mode_vec.end()
);

const std::unordered_map<MessageType, std::string> s_message_type_to_str = {
  { MessageType::INVALID, "INVALID" },
  { MessageType::LOGIN, "LOGIN" },
  { MessageType::QUIT, "QUIT" },
  { MessageType::ORDER_NEW, "ORDER_NEW" },
  { MessageType::ITEM_UPDATE, "ITEM_UPDATE" },
  { MessageType::ORDER_UPDATE, "ORDER_UPDATE" },
  { MessageType::OK, "OK" },
  { MessageType::ERROR, "ERROR" },
  { MessageType::DISP_ORDER, "DISP_ORDER" },
  { MessageType::DISP_ITEM_UPDATE, "DISP_ITEM_UPDATE" },
  { MessageType::DISP_ORDER_UPDATE, "DISP_ORDER_UPDATE" },
  { MessageType::DISP_HEARTBEAT, "DISP_HEARTBEAT" },
};

const std::vector<std::pair<std::string, MessageType>> s_str_to_message_type_vec =
  Util::invert_unordered_map(s_message_type_to_str);

const std::unordered_map<std::string, MessageType> s_str_to_message_type(
  s_str_to_message_type_vec.begin(),
  s_str_to_message_type_vec.end()
);

const std::unordered_map<ItemStatus, std::string> s_item_status_to_str = {
  { ItemStatus::INVALID, "INVALID" },
  { ItemStatus::NEW, "NEW" },
  { ItemStatus::IN_PROGRESS, "IN_PROGRESS" },
  { ItemStatus::DONE, "DONE" },
};

const std::vector<std::pair<std::string, ItemStatus>> s_str_to_item_status_vec =
  Util::invert_unordered_map(s_item_status_to_str);

const std::unordered_map<std::string, ItemStatus> s_str_to_item_status(
  s_str_to_item_status_vec.begin(),
  s_str_to_item_status_vec.end()
);

const std::unordered_map<OrderStatus, std::string> s_order_status_to_str = {
  { OrderStatus::INVALID, "INVALID" },
  { OrderStatus::NEW, "NEW" },
  { OrderStatus::IN_PROGRESS, "IN_PROGRESS" },
  { OrderStatus::DONE, "DONE" },
  { OrderStatus::DELIVERED, "DELIVERED" },
};

const std::vector<std::pair<std::string, OrderStatus>> s_str_to_order_status_vec =
  Util::invert_unordered_map(s_order_status_to_str);

const std::unordered_map<std::string, OrderStatus> s_str_to_order_status(
  s_str_to_order_status_vec.begin(),
  s_str_to_order_status_vec.end()
);

// helper function to encode order 
const std::string order_to_str(const Order &order) {
  if (order.get_num_items() == 0) {
      throw InvalidMessage("order has no items");
  }

  // order format: order id,order status,item list 
  std::string s = std::to_string(order.get_id()) + ","; 
  s += Wire::order_status_to_str(order.get_status()) + ","; 
  
  // convert each item in the item list to str 
  // item list format: order id:item id:item status:description string:integer quantity; 
  for (int i = 0; i < order.get_num_items(); i++) {
    if (i != 0) {
      s += ";"; 
    }

    auto item = order.at(i); 
    std::string item_list = std::to_string(item->get_order_id()) + ":"; 
    item_list += std::to_string(item->get_id()) + ":"; 
    item_list += Wire::item_status_to_str(item->get_status()) + ":"; 
    item_list += item->get_desc() + ":"; 
    item_list += std::to_string(item->get_qty()); 

    s += item_list; 
  }

  return s; 
}

// helper function to check if string is int 
bool is_pos_int(const std::string s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit) && std::stoi(s) != 0;
}

// helper function to check no element in vec is empty 
bool no_empty_str(const std::vector<std::string>& vec) {
  for (size_t i = 0; i < vec.size(); i++) {
    if (vec[i].empty()) {
      throw InvalidMessage("expected field is empty"); 
    }
  }
  return true; 
}

// helper function to split string into vector with a given char separator
std::vector<std::string> str_to_vec(const std::string s, const char c) {
  std::stringstream ss(s); 
  std::string token; 
  std::vector<std::string> vec; 
  while (std::getline(ss, token, c)) {
    vec.push_back(token); 
  }

  if (vec.empty() || !no_empty_str(vec)) {
    throw InvalidMessage("expected field is empty"); 
  }

  return vec; 
}

// helper function to decode item 
std::shared_ptr<Item> decode_item(std::string s, int order_id) {
  // split item into vector separated by ':'
  auto item = str_to_vec(s, ':'); 

  // item: [order id, item id, item status, description string, integer quantity]
  if (item.size() != 5) {
    throw InvalidMessage("item has incorrect number of fields"); 
  }

  // order id 
  // check item's order id matches actual order id 
  if (!is_pos_int(item[0]) || std::stoi(item[0]) != order_id) {
    throw InvalidMessage("invalid order id"); 
  }
  
  // item id 
  if (!is_pos_int(item[1])) {
    throw InvalidMessage("invalid item id"); 
  }
  int item_id = std::stoi(item[1]); 

  // item status 
  ItemStatus item_status = Wire::str_to_item_status(item[2]); 
  if (item_status == ItemStatus::INVALID) {
    throw InvalidMessage("invalid item status"); 
  }

  // description string 
  std::string desc = item[3]; 

  // integer quantity 
  if (!is_pos_int(item[4])) {
    throw InvalidMessage("invalid item quantity"); 
  }
  int qty = std::stoi(item[4]); 

  // elements valid so create and return item ptr 
  return std::make_shared<Item>(order_id, item_id, item_status, desc, qty);
}

// helper function to decode login messagetype 
void decode_login(Message &msg, const std::vector<std::string> &vec) {
  // vec: [type, client mode, string]
  if (vec.size() != 3) {
    throw InvalidMessage("message has incorrect number of fields"); 
  }

  // client mode 
  ClientMode client_mode = Wire::str_to_client_mode(vec[1]); 
  if (client_mode == ClientMode::INVALID) {
    throw InvalidMessage("message has invalid client mode"); 
  }
  msg.set_client_mode(client_mode); 

  // string 
  msg.set_str(vec[2]); 
}

// helper function to decode quit/ok/error messagetype
void decode_quit_ok_error(Message &msg, const std::vector<std::string> &vec) {
  // vec: [type, string]
  if (vec.size() != 2) {
    throw InvalidMessage("message has incorrect number of fields"); 
  }

  // string 
  msg.set_str(vec[1]); 
}

// helper function to decode order_new/disp_order messagetype 
void decode_ordernew_disporder(Message &msg, const std::vector<std::string> &vec) {
  // vec: [type, order]
  if (vec.size() != 2) {
    throw InvalidMessage("message has incorrect number of fields"); 
  }

  // order 
  // split order into vector separated by ','
  auto order_vec = str_to_vec(vec[1], ','); 

  // order_vec: [order id, order status, item list]
  if (order_vec.size() != 3) {
    throw InvalidMessage("order has incorrect number of fields"); 
  }

  // order id 
  if (!is_pos_int(order_vec[0])) {
    throw InvalidMessage("invalid order id"); 
  }
  int order_id = std::stoi(order_vec[0]); 

  // order status 
  OrderStatus order_status = Wire::str_to_order_status(order_vec[1]); 
  if (order_status == OrderStatus::INVALID) {
    throw InvalidMessage("invalid order status"); 
  }

  // elements valid so create order ptr 
  auto order = std::make_shared<Order>(order_id, order_status); 

  // split item list into vector separated by ';""
  auto item_list = str_to_vec(order_vec[2], ';'); 

  // item_list: [item1, item2, etc.]
  for (size_t i = 0; i < item_list.size(); i++) {
    // add each item to order 
    order->add_item(decode_item(item_list[i], order_id)); 
  }

  // add order populated with all items to msg 
  msg.set_order(order); 
}

// helper function to decode item_update/disp_item_update messagetype 
void decode_itemupdate_dispitemupdate(Message &msg, const std::vector<std::string> &vec) {
  // vec: [type, order id, item id, item status]
  if (vec.size() != 4) {
    throw InvalidMessage("message has incorrect number of fields"); 
  }

  // order id 
  if (!is_pos_int(vec[1])) {
    throw InvalidMessage("invalid order id"); 
  }
  msg.set_order_id(std::stoi(vec[1])); 

  // item id 
  if (!is_pos_int(vec[2])) {
    throw InvalidMessage("invalid item id"); 
  }
  msg.set_item_id(std::stoi(vec[2])); 

  // item_status 
  ItemStatus item_status = Wire::str_to_item_status(vec[3]); 
  if (item_status == ItemStatus::INVALID) {
    throw InvalidMessage("invalid item status"); 
  }
  msg.set_item_status(item_status); 
}

// helper function to decode order_update/disp_order_update messagetype 
void decode_orderupdate_disporderupdate(Message &msg, const std::vector<std::string> &vec) {
  // vec: [type, order id, order status]
  if (vec.size() != 3) {
    throw InvalidMessage("message has incorrect number of fields"); 
  }

  // order id 
  if (!is_pos_int(vec[1]) ) {
    throw InvalidMessage("invalid order id"); 
  }
  msg.set_order_id(std::stoi(vec[1])); 

  // order status 
  OrderStatus order_status = Wire::str_to_order_status(vec[2]); 
  if (order_status == OrderStatus::INVALID) {
    throw InvalidMessage("invalid order status"); 
  }
  msg.set_order_status(order_status); 
}

} // end of anonymous namespace for helper functions

namespace Wire {

std::string client_mode_to_str(ClientMode client_mode) {
  auto i = s_client_mode_to_str.find(client_mode);
  return (i != s_client_mode_to_str.end()) ? i->second : "<invalid>";
}

ClientMode str_to_client_mode(const std::string &s) {
  auto i = s_str_to_client_mode.find(s);
  return (i != s_str_to_client_mode.end()) ? i->second : ClientMode::INVALID;
}

std::string message_type_to_str(MessageType message_type) {
  auto i = s_message_type_to_str.find(message_type);
  return (i != s_message_type_to_str.end()) ? i->second : "<invalid>";
}

MessageType str_to_message_type(const std::string &s) {
  auto i = s_str_to_message_type.find(s);
  return (i != s_str_to_message_type.end()) ? i->second : MessageType::INVALID;
}

std::string item_status_to_str(ItemStatus item_status) {
  auto i = s_item_status_to_str.find(item_status);
  return (i != s_item_status_to_str.end()) ? i->second : "<invalid>";
}

ItemStatus str_to_item_status(const std::string &s) {
  auto i = s_str_to_item_status.find(s);
  return (i != s_str_to_item_status.end()) ? i->second : ItemStatus::INVALID;
}

std::string order_status_to_str(OrderStatus order_status) {
  auto i = s_order_status_to_str.find(order_status);
  return (i != s_order_status_to_str.end()) ? i->second : "<invalid>";
}

OrderStatus str_to_order_status(const std::string &s) {
  auto i = s_str_to_order_status.find(s);
  return (i != s_str_to_order_status.end()) ? i->second : OrderStatus::INVALID;
}

void encode(const Message &msg, std::string &s) {
  s = message_type_to_str(msg.get_type());

  if (msg.has_client_mode()) {
    s += "|" + client_mode_to_str(msg.get_client_mode()); 
  } 
  
  if (msg.has_str()) {
    s += "|" + msg.get_str(); 
  }

  if (msg.has_order()) {
    s += "|" + order_to_str(*msg.get_order()); 
  }

  if (msg.has_order_id()) {
    s += "|" + std::to_string(msg.get_order_id()); 
  }

  if (msg.has_order_status()) {
    s += "|" + Wire::order_status_to_str(msg.get_order_status()); 
  }

  if (msg.has_item_id()) {
    s += "|" + std::to_string(msg.get_item_id()); 
  }

  if (msg.has_item_status()) {
    s += "|" + Wire::item_status_to_str(msg.get_item_status()); 
  }
}

void decode(const std::string &s, Message &msg) {
  // split string into vector separated by '|' 
  auto vec = str_to_vec(s, '|'); 

  // set message type 
  MessageType type = str_to_message_type(vec[0]); 
  if (type == MessageType::INVALID) {
    throw InvalidMessage("invalid message type"); 
  }
  msg.set_type(type); 

  // call helper based on contained data values 
  if (type == MessageType::LOGIN) {
    decode_login(msg, vec); 
  } else if (type == MessageType::QUIT || type == MessageType::OK || type == MessageType::ERROR) {
    decode_quit_ok_error(msg, vec); 
  } else if (type == MessageType::ORDER_NEW || type == MessageType::DISP_ORDER) {
    decode_ordernew_disporder(msg, vec); 
  } else if (type == MessageType::ITEM_UPDATE || type == MessageType::DISP_ITEM_UPDATE) {
    decode_itemupdate_dispitemupdate(msg, vec); 
  } else if (type == MessageType::ORDER_UPDATE || type == MessageType::DISP_ORDER_UPDATE) {
    decode_orderupdate_disporderupdate(msg, vec); 
  } else if (type == MessageType::DISP_HEARTBEAT) {
    return; 
  } else {
    throw InvalidMessage("invalid encoded message"); 
  }
}

}
