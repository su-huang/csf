#include <sstream>
#include <iomanip>
#include <cassert>
#include <string>
#include <algorithm> 
#include <vector> 
#include "csapp.h"
#include "except.h"
#include "io.h"

namespace {

// helper function to frame the message 
std::string frame(const std::string &s) {
  std::stringstream ss; 
  ss << std::setw(4) << std::setfill('0') << (s.length() + 1); 
  return ss.str() + s + "\n"; 
}

} // end of anonymous namespace for helper functions

namespace IO {

void send(const std::string &s, int outfd) {
  // frame message 
  std::string framed = frame(s); 
  
  // write to socket
  if (rio_writen(outfd, framed.c_str(), framed.size()) < 0)
    throw IOException("error writing");
}

void receive(int infd, std::string &s) {
  // read length value 
  std::vector<char> length_vec(4); 
  if (rio_readn(infd, length_vec.data(), 4) != 4) {
    throw IOException("error reading length value"); 
  }

  // verify length value 
  for (size_t i = 0; i < 4; i++) {
    if (!isdigit(length_vec[i])) {
      throw IOException("invalid length value"); 
    }
  }

  int length_value = std::stoi(std::string(length_vec.data(), 4));
  if (length_value < 1) {
    throw IOException("invalid length value"); 
  }
  
  // read length_value bytes 
  std::vector<char> msg(length_value); 
  if (rio_readn(infd, msg.data(), length_value) != length_value) {
    throw IOException("error reading message"); 
  }

  if (msg.back() != '\n') {
    throw IOException("message missing newline"); 
  }

  s = std::string(msg.data(), length_value - 1); 
}

} // end of IO namespace
