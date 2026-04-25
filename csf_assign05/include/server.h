#ifndef SERVER_H
#define SERVER_H

#include <map>
#include <unordered_set>
#include <pthread.h>
#include <vector> 
#include "util.h"
#include "model.h"

// Forward declarations
class Client;
class Message;

class Server {
private:
  // Private data
  pthread_mutex_t m_mutex;  // synchronization construct to guarantee mutual exclusion 
  int m_next_order_id;      // track next order id
  std::map<int, std::shared_ptr<Order>> m_orders_map;   // maps order id to order objects 
  std::unordered_set<Client*> m_display_clients_set;              // set of active display clients 

  // no value semantics
  NO_VALUE_SEMANTICS(Server);

public:
  Server();
  ~Server();

  // Create a server socket listening on specified port,
  // and accept connections from clients. This function does
  // not return.
  void server_loop(const char *port);

  // generate new order 
  std::shared_ptr<Order> order_new(std::shared_ptr<Order> order); 

  // update order 
  void order_update(int order_id, OrderStatus new_order_status); 

  // update item
  void item_update(int order_id, int item_id, ItemStatus new_item_status); 

  // add display client 
  void add_display_client(Client *client); 

  // remove display client 
  void remove_display_client(Client *client); 

  // return vector of orders 
  std::vector<std::shared_ptr<Order>> get_all_orders(); 


private:
  // helper function to check valid order id (return null if invalid)
  std::shared_ptr<Order> valid_order_id(int order_id) const ; 

  // helper function to validate order status transitions 
  bool valid_next_order_status(OrderStatus curr_status, OrderStatus next_status) const; 

  // helper funciton to validate item status transitions 
  bool valid_next_item_status(ItemStatus curr_status, ItemStatus next_status) const; 

  // helper function to check if all items are new except given item in progress 
  bool first_in_progress(const std::shared_ptr<Order> order, int item_id) const; 
  
  // helper function to check if all items are done 
  bool all_items_done(const std::shared_ptr<Order> order) const; 

  // update order when mutex already locked 
  void order_update_internal(const std::shared_ptr<Order> order, OrderStatus new_order_status); 

  // queue message to broadcast to display clients 
  void broadcast(const std::shared_ptr<Message> msg) const; 
};

#endif // SERVER_H
