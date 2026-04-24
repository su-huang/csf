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

// TODO: you can define standalone helper functions here
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
  m_next_order_id = 1000; 
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

// TODO: other member functions

