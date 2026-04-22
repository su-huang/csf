#ifndef CLIENT_UTIL_H
#define CLIENT_UTIL_H

#include "message.h"
#include "wire.h"
#include "io.h"

// TODO: classes and functions used by the client program
// helper function to send a message and return the received message 
Message send_recv(int fd, const Message &msg); 

// This is a way that you can avoid code duplication between
// the updater and display clients

#endif // CLIENT_UTIL_H
