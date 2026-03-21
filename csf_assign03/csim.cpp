#include <iostream>
#include <string>
#include <cmath>
#include "csim.h"

bool check_params(int argc, char* argv[], Config& config) {
    if (argc != 7) {
        std::cerr << "Error: Incorrect number of arguments" << std::endl;
        return false;
    }

    try {
        config.num_sets = static_cast<uint32_t>(std::stoul(argv[1])); 
        config.num_blocks = static_cast<uint32_t>(std::stoul(argv[2])); 
        config.num_bytes = static_cast<uint32_t>(std::stoul(argv[3])); 

        if (!is_power_of_two(config.num_sets)) {
            std::cerr << "Error: Number of sets must be a power of 2" << std::endl;
            return false;
        }
        if (!is_power_of_two(config.num_blocks)) {
            std::cerr << "Error: Number of blocks must be a power of 2" << std::endl;
            return false;
        }
        if (!is_power_of_two(config.num_bytes) || config.num_bytes < 4) {
            std::cerr << "Error: Block size must be a power of 2 and at least 4 bytes" << std::endl;
            return false;
        }
    } catch (...) {
        std::cerr << "Error: Expected numeric arguments" << std::endl;
        return false;
    }

    std::string write_alloc = argv[4]; 
    if (write_alloc == "write-allocate") config.write_allocate = true; 
    else if (write_alloc == "no-write-allocate") config.write_allocate = false; 
    else {
        std::cerr << "Error: Expected 'write-allocate' or 'no-write-allocate'" << std::endl; 
        return false; 
    }

    std::string write_through = argv[5]; 
    if (write_through == "write-through") config.write_through = true; 
    else if (write_through == "write-back") config.write_through = false; 
    else {
        std::cerr << "Error: Expected 'write-through' or 'write-back'" << std::endl; 
        return false; 
    }

    if (!config.write_allocate && !config.write_through) {
        std::cerr << "Error: Doesn’t make sense to combine no-write-allocate with write-back" << std::endl; 
        return false; 
    }

    std::string evict_type = argv[6]; 
    if (evict_type == "lru") config.lru = true; 
    else if (evict_type == "fifo") config.lru = false;
    else {
        std::cerr << "Error: Expected 'lru' or 'fifo'" << std::endl; 
        return false; 
    }

    return true; 
}

bool is_power_of_two(uint32_t n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

uint32_t get_bits(uint32_t n) {
    uint32_t bits = 0;
    while (n >>= 1) {
        bits++;
    }
    return bits;
}

uint32_t get_index(const Config& config, uint32_t address) {
    uint32_t offset = get_bits(config.num_bytes);
    return (address >> offset) & (config.num_sets - 1);
}

uint32_t get_tag(const Config& config, uint32_t address) {
    uint32_t offset = get_bits(config.num_bytes);
    uint32_t index = get_bits(config.num_sets);
    return address >> (offset + index);
}

void run_simulation(Cache& cache) {}

void access(Cache& cache, char type, uint32_t address) {}

int find_hit(const Set& set, uint32_t tag) {
    for (size_t i = 0; i < set.slots.size(); i++) {
        if (set.slots[i].valid && set.slots[i].tag == tag) return static_cast<int>(i); 
    }
    return -1; 
}

int find_evict_index(const Set& set, bool lru) {}

void print_stats(const Stats& stats) {}