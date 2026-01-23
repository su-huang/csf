#include <cassert>
#include "bigint.h"
#include <sstream> 
#include <iomanip> 
#include <iostream> 

BigInt::BigInt() {
  // TODO: initialize member variables 
  magnitude = {}; 
  negative = false; 
}

BigInt::BigInt(uint64_t val, bool negative) {
  // TODO: initialize member variables 
  if (val == 0) {
    magnitude = {}; 
  } else {
    magnitude.push_back(val); 
  }
  this->negative = negative; 
}

BigInt::BigInt(std::initializer_list<uint64_t> vals, bool negative) {
  // TODO: initialize member variables 
  for (auto val : vals) {
    magnitude.push_back(val); 
  }
  this->negative = negative; 
}

BigInt::BigInt(const BigInt &other) {
  // TODO: initialize member variables
  magnitude = other.magnitude; 
  negative = other.negative; 
}

BigInt::~BigInt() {
}

BigInt &BigInt::operator=(const BigInt &rhs)
{
  // TODO: implement
}

bool BigInt::is_negative() const {
  // TODO: implement
  return negative; 
}

const std::vector<uint64_t> &BigInt::get_bit_vector() const {
  // TODO: implement
  return magnitude; 
}

uint64_t BigInt::get_bits(unsigned index) const {
  // TODO: implement
  if (index >= magnitude.size()) {
    return 0; 
  }
  return magnitude[index]; 
}

BigInt BigInt::operator+(const BigInt &rhs) const {
  // TODO: implement
}

BigInt BigInt::operator-(const BigInt &rhs) const {
  // TODO: implement
  // Hint: a - b could be computed as a + -b
}

BigInt BigInt::operator-() const {
  // TODO: implement
  BigInt ans = *this; 
  if (!is_zero()) {
    ans.negative = !this->negative; 
  }
  return ans; 
}

bool BigInt::is_bit_set(unsigned n) const
{
  // TODO: implement
}

BigInt BigInt::operator<<(unsigned n) const
{
  // TODO: implement
}

BigInt BigInt::operator*(const BigInt &rhs) const
{
  // TODO: implement
}

BigInt BigInt::operator/(const BigInt &rhs) const
{
  // TODO: implement
}

int BigInt::compare(const BigInt &rhs) const
{
  // TODO: implement
}

std::string BigInt::to_hex() const {
  // TODO: implement
  if (is_zero()) {
    return "0"; 
  }

  std::stringstream ss; 
  bool first = true; 

  if (is_negative()) {
    ss << "-"; 
  }

  for (auto val = magnitude.rbegin(); val != magnitude.rend(); ++val) {
    if (first) {
      ss << std::hex << *val; 
      first = false; 
    } else {
      ss << std::hex << std::setfill('0') << std::setw(16) << *val; 
    }
  }

  return ss.str(); 
}

std::string BigInt::to_dec() const
{
  // TODO: implement
}

bool BigInt::is_zero() const {
  return this->magnitude.empty(); 
} 

