#ifndef CSIM_H
#define CSIM_H

#include <stdint.h>
#include <vector> 

struct Config {
    uint32_t num_sets;      // positive power-of-2
    uint32_t num_blocks;    // positive power-of-2
    uint32_t num_bytes;     // positive power-of-2
    bool write_allocate;    // true if write-allocate, false if no-write-allocate
    bool write_through;     // true if write-through, false if write-back 
    bool lru;               // true if lru, false if fifo 
}; 

struct Stats {
    uint32_t total_loads = 0; 
    uint32_t total_stores = 0; 
    uint32_t load_hits = 0; 
    uint32_t load_misses = 0; 
    uint32_t store_hits = 0; 
    uint32_t store_misses = 0; 
    uint64_t total_cycles = 0; 
}; 

struct Slot {
    uint32_t tag; 
    bool valid; 
    bool dirty; 
    uint32_t load_ts; 
    uint32_t access_ts; 
}; 

struct Set {
    std::vector<Slot> slots; 
}; 

#endif