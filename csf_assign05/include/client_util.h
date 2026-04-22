/*
 * Header for client util
 * CSF Assignment 5 
 * Su Huang 
 * shuan148@jh.edu
 */

#ifndef CLIENT_UTIL_H
#define CLIENT_UTIL_H

#include <string> 
#include <iostream> 
#include "message.h"
#include "wire.h"
#include "io.h"
#include "except.h"

// helper function to send a message and return the received message 
Message send_recv(int fd, const Message &msg); 

// helper function to prompt user login and return login status 
bool login(int fd, ClientMode client_mode); 

// This is a way that you can avoid code duplication between
// the updater and display clients

#endif // CLIENT_UTIL_H
