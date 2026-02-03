/*
 * Complete implementation for Big Integers
 * CSF Assignment 1 
 * Su Huang 
 * shuan148@jhu.edu
 */
 
#include <cassert>
#include "bigint.h"
#include <sstream> 
#include <iomanip> 
#include <iostream> 
#include <algorithm>

BigInt::BigInt() {
  // Empty vector represents 0 
  magnitude = {}; 
  negative = false; 
}

BigInt::BigInt(uint64_t val, bool negative) {
  if (val == 0) {
    // Empty vector represents 0 
    magnitude = {};   
    this->negative = false; 
  } else {
    magnitude.push_back(val); 
    this->negative = negative; 
  }
}

BigInt::BigInt(std::initializer_list<uint64_t> vals, bool negative) {
  for (auto val : vals) {
    magnitude.push_back(val); 
  }
  this->negative = negative;
  clean(); 
}

BigInt::BigInt(const BigInt &other) {
  magnitude = other.magnitude; 
  negative = other.negative; 
}

BigInt::~BigInt() {
}

BigInt &BigInt::operator=(const BigInt &rhs) {
  magnitude = rhs.magnitude; 
  negative = rhs.negative; 

  return *this; 
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
  BigInt ans;
  if (negative == rhs.negative) { 
    ans.negative = negative; 
    uint64_t carry = 0; 

    for (unsigned index = 0; index < magnitude.size() || index < rhs.magnitude.size(); index++) {
      ans.magnitude.push_back(add_magnitudes(get_bits(index), rhs.get_bits(index), carry)); 
    }

    if (carry) {
      ans.magnitude.push_back(carry); 
    }
  } else {
    const BigInt &larger = (compare_magnitude(rhs) < 0) ? rhs : *this;
    const BigInt &smaller = (compare_magnitude(rhs) < 0) ? *this : rhs;
    ans.negative = larger.negative; 
    uint64_t borrow = 0; 

    for (unsigned index = 0; index < larger.magnitude.size(); index++) {
      ans.magnitude.push_back(subtract_magnitudes(larger.get_bits(index), smaller.get_bits(index), borrow)); 
    }
  }

  ans.clean(); 
  
  return ans; 
}

BigInt BigInt::operator-(const BigInt &rhs) const {
  return *this + (-rhs); 
}

BigInt BigInt::operator-() const {
  BigInt ans = *this; 
  // 0 must remain non-negative 
  if (!is_zero()) {
    ans.negative = !this->negative; 
  }

  return ans; 
}

bool BigInt::is_bit_set(unsigned n) const {
  unsigned unit_index = n / 64; 
  unsigned bit_index = n % 64; 

  if (unit_index >= magnitude.size()) {
    return false; 
  }

  return ((magnitude[unit_index] >> bit_index) & 1) == 1; 
}

BigInt BigInt::operator<<(unsigned n) const {
  if (negative) {
    throw std::invalid_argument("Cannot perform left shift on negative values"); 
  }

  if (is_zero() || n == 0) {
    return *this; 
  }

  BigInt ans; 
  unsigned units = n / 64; 
  unsigned bits = n % 64; 

  for (unsigned i = 0; i < units; i++) {
    ans.magnitude.push_back((uint64_t) 0); 
  }

  uint64_t carry = 0; 
  for (auto curr : magnitude) {
    ans.magnitude.push_back((curr << bits) | carry); 
    if (bits != 0) {
      carry = curr >> (64 - bits); 
    } 
  }

  if (carry) {
    ans.magnitude.push_back(carry); 
  }

  ans.negative = false; 
  ans.clean(); 
  return ans; 
}

BigInt BigInt::operator*(const BigInt &rhs) const {
  if (is_zero() || rhs.is_zero()) {
    return BigInt(); 
  }

  BigInt ans;  

  // Create a non-negative copy so we can perform left shift 
  BigInt other = rhs; 
  other.negative = false; 

  unsigned bits = magnitude.size() * 64; 
  for (unsigned i = 0; i < bits; i++) {
    if (is_bit_set(i)) {
      ans = ans + (other << i); 
    }
  }

  ans.negative = (negative != rhs.negative);
  ans.clean(); 
  return ans; 
}

BigInt BigInt::operator/(const BigInt &rhs) const {
  if (rhs.is_zero()) {
    throw std::invalid_argument("Cannot divide by 0"); 
  }

  if (is_zero()) {
    return BigInt(); 
  }

  // Make unsigned copy to prevent sign implications 
  BigInt n = *this; 
  n.negative = false; 
  BigInt m = rhs; 
  m.negative = false; 

  BigInt ans; 
  
  BigInt low(0);
  BigInt high = *this; 
  high.negative = false; 
  
  while (low.compare(high) <= 0) {
    BigInt mid = (low + high).right_shift(); 
    BigInt product = mid * m; 

    int comp = product.compare(n); 

    if (comp <= 0) {
      ans = mid; 
      low = mid + BigInt(1); 
    } else {
      high = mid - BigInt(1); 
    }
  }

  ans.negative = (negative != rhs.negative); 
  ans.clean(); 
  return ans; 
}

int BigInt::compare(const BigInt &rhs) const {
  if (negative != rhs.negative) {
    return (negative ? -1 : 1); 
  }

  return (negative ? -compare_magnitude(rhs) : compare_magnitude(rhs)); 
}

std::string BigInt::to_hex() const {
  if (is_zero()) {
    return "0"; 
  }

  std::stringstream ss; 
  bool first = true; 

  if (negative) {
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

std::string BigInt::to_dec() const {
  if (is_zero()) {
    return "0"; 
  }

  std::stringstream ss; 
  BigInt num = *this; 
  num.negative = false; 
  BigInt ten(10); 

  while (!num.is_zero()) {
    BigInt q = num / ten; 
    BigInt r = num - (ten * q); 
    ss << r.get_bits(0); 
    num = q; 
  }

  if (negative) {
    ss << "-"; 
  }

  std::string ans = ss.str();  
  std::reverse(ans.begin(), ans.end()); 
  return ans; 
}

bool BigInt::is_zero() const {
  // 0 is stored as an empty vector 
  return magnitude.empty(); 
} 

uint64_t BigInt::add_magnitudes(uint64_t a, uint64_t b, uint64_t &carry) const {
  uint64_t sum = a + b + carry; 
  if (sum < a || (carry && sum == a)) {
    carry = 1; 
  } else {
    carry = 0; 
  }

  return sum; 
}

uint64_t BigInt::subtract_magnitudes(uint64_t a, uint64_t b, uint64_t &borrow) const {
  uint64_t difference = a - b - borrow; 
  if (a - borrow < b) {
    borrow = 1; 
  } else {
    borrow = 0; 
  }

  return difference; 
}

int BigInt::compare_magnitude(const BigInt &rhs) const { 
  if (magnitude.size() > rhs.magnitude.size()) {
    return 1; 
  } else if (magnitude.size() < rhs.magnitude.size()) {
    return -1; 
  }

  for (int index = magnitude.size() - 1; index >= 0; index--) {
    if (get_bits(index) != rhs.get_bits(index)) {
      return (get_bits(index) > rhs.get_bits(index) ? 1 : -1); 
    }
  }

  return 0; 
}

void BigInt::clean() {
  while (!magnitude.empty() && magnitude.back() == 0) {
    magnitude.pop_back(); 
  }

  if (magnitude.empty()) {
    negative = false; 
  }
} 

BigInt BigInt::right_shift() const {
  if (is_zero()) {
    return *this; 
  }

  BigInt ans = *this; 
  uint64_t carry = 0; 

  // Iterate from MSB to LSB 
  for (int i = magnitude.size() - 1; i >= 0; i--) {
    ans.magnitude[i] = (magnitude[i] >> 1) | carry; 
    carry = (magnitude[i] & 1) ? ((uint64_t) 1 << 63) : 0; 
  }

  ans.clean(); 
  return ans; 
}

