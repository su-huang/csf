#include <cassert>
#include "bigint.h"
#include <sstream> 
#include <iomanip> 
#include <iostream> 

BigInt::BigInt() {
  // Empty vector represents 0 
  magnitude = {}; 
  negative = false; 
}

BigInt::BigInt(uint64_t val, bool negative) {
  if (val == 0) {
    // Empty vector represents 0 
    magnitude = {};   
  } else {
    magnitude.push_back(val); 
  }

  this->negative = negative; 
}

BigInt::BigInt(std::initializer_list<uint64_t> vals, bool negative) {
  for (auto val : vals) {
    magnitude.push_back(val); 
  }

  this->negative = negative; 
}

BigInt::BigInt(const BigInt &other) {
  magnitude = other.magnitude; 
  negative = other.negative; 
}

BigInt::~BigInt() {
}

BigInt &BigInt::operator=(const BigInt &rhs) {
  // TODO: implement
}

bool BigInt::is_negative() const {
  return negative; 
}

const std::vector<uint64_t> &BigInt::get_bit_vector() const {
  return magnitude; 
}

uint64_t BigInt::get_bits(unsigned index) const {
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
  BigInt ans = *this; 
  // 0 must keep positive sign 
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
      // No leading 0 digits 
      ss << std::hex << *val; 
      first = false; 
    } else {
      // Pad with 0s to 16 digits  
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
  // 0 is stored as an empty vector 
  return magnitude.empty(); 
} 

