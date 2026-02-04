/*
 * Complete implementation for Big Integers
 * CSF Assignment 1 
 * Su Huang 
 * shuan148@jh.edu
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

  // Add magnitude of each uint_64 if both have the same sign
  if (negative == rhs.negative) { 
    ans.negative = negative; 
    uint64_t carry = 0; 

    for (unsigned i = 0; i < magnitude.size() || i < rhs.magnitude.size(); i++) {
      // get_bits() ensures that we get 0 for out of range indices 
      ans.magnitude.push_back(add_magnitudes(get_bits(i), rhs.get_bits(i), carry)); 
    }

    // Add left-over carry 
    if (carry) {
      ans.magnitude.push_back(carry); 
    }
  } else {
    // Subtracting each smaller uint_64 magnitude from larger uint_64 if values have different signs 
    const BigInt &larger = (compare_magnitude(rhs) < 0) ? rhs : *this;
    const BigInt &smaller = (compare_magnitude(rhs) < 0) ? *this : rhs;

    ans.negative = larger.negative; 
    uint64_t borrow = 0; 

    for (unsigned i = 0; i < larger.magnitude.size(); i++) {
      // get_bits() ensures that we get 0 for out of range indices 
      ans.magnitude.push_back(subtract_magnitudes(larger.get_bits(i), smaller.get_bits(i), borrow)); 
    }
  }

  ans.clean(); 
  return ans; 
}

BigInt BigInt::operator-(const BigInt &rhs) const {
  // Negate then add 
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
  // Separate n into index in vector and bit in uint_64
  unsigned unit_index = n / 64; 
  unsigned bit_index = n % 64; 

  if (unit_index >= magnitude.size()) {
    return false; 
  }

  // Isolates the desired bit 
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

  // Separate n into whole uint_64 to shift and bits 
  unsigned units = n / 64; 
  unsigned bits = n % 64; 

  // Shifting whole uint_64 blocks 
  for (unsigned i = 0; i < units; i++) {
    ans.magnitude.push_back((uint64_t) 0); 
  }

  uint64_t carry = 0; 
  for (auto curr : magnitude) {
    // Shift by bits and insert carry 
    ans.magnitude.push_back((curr << bits) | carry); 

    // Save lost bits from shift 
    carry = (bits == 0) ? 0 : (curr >> (64 - bits)); 
  }

  // Append left-over carry 
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

  for (unsigned i = 0; i < magnitude.size() * 64; i++) {
    // Bits = 1 indicate a factor of a power of 2
    if (is_bit_set(i)) {
      // Left shift by i to multiply by 2^i
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

  // Make unsigned copy to prevent sign implications in comparisons 
  BigInt n = *this; 
  n.negative = false; 
  BigInt m = rhs; 
  m.negative = false; 

  BigInt ans; 
  
  // Set low and high integers for binary search 
  BigInt low(0);
  BigInt high = *this; 
  high.negative = false; 
  
  // Perform inclusive binary search to narrow-down quotient 
  while (low.compare(high) <= 0) {
    BigInt mid = (low + high).right_shift(); 
    BigInt product = mid * m; 

    int comp = product.compare(n); 

    // Truncate so save smaller value even if not exact 
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
  // First compare signs
  if (negative != rhs.negative) {
    return (negative ? -1 : 1); 
  }

  // Compare magnitudes 
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
  BigInt ten(10); 

  // Create a copy so we don't need to worry about signs 
  BigInt num = *this; 
  num.negative = false; 

  // Build decimal from least significant to most significant 
  // Divide by 10 and track remainder until 0 
  while (!num.is_zero()) {
    BigInt q = num / ten; 
    BigInt r = num - (ten * q); 
    ss << r.get_bits(0); 
    num = q; 
  }

  if (negative) {
    ss << "-"; 
  }

  // Reverse string to get digits in right order 
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

  // Update carry if bit lost from incorrect sum (overflow)
  if (sum < a || (carry && sum == a)) {
    carry = 1; 
  } else {
    carry = 0; 
  }

  return sum; 
}

uint64_t BigInt::subtract_magnitudes(uint64_t a, uint64_t b, uint64_t &borrow) const {
  uint64_t difference = a - b - borrow; 

  // Update borrow if bit lost from incorrect difference (underflow)
  if (a - borrow < b) {
    borrow = 1; 
  } else {
    borrow = 0; 
  }

  return difference; 
}

int BigInt::compare_magnitude(const BigInt &rhs) const { 
  // Compare number of unint_64 
  if (magnitude.size() > rhs.magnitude.size()) {
    return 1; 
  } else if (magnitude.size() < rhs.magnitude.size()) {
    return -1; 
  }

  // Compare from most significant to least significant bit 
  for (int index = magnitude.size() - 1; index >= 0; index--) {
    if (get_bits(index) != rhs.get_bits(index)) {
      return (get_bits(index) > rhs.get_bits(index) ? 1 : -1); 
    }
  }

  return 0; 
}

void BigInt::clean() {
  // Remove trailing 0s 
  while (!magnitude.empty() && magnitude.back() == 0) {
    magnitude.pop_back(); 
  }

  // Value is 0 
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

  // Iterate from mosft significant to least significant bit 
  for (int i = magnitude.size() - 1; i >= 0; i--) {
    // Shift and insert carry from more significant uint_64 
    ans.magnitude[i] = (magnitude[i] >> 1) | carry; 

    // Save least significant bit from uint_64 of original integer 
    carry = (magnitude[i] & 1) ? ((uint64_t) 1 << 63) : 0; 
  }

  ans.clean(); 
  return ans; 
}

