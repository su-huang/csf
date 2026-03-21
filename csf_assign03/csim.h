#ifndef CSIM_H
#define CSIM_H

#include <stdint.h>
#include <vector>
#include <string>

struct Config {
    uint32_t num_sets;      // positive power-of-2
    uint32_t num_blocks;    // positive power-of-2
    uint32_t num_bytes;     // positive power-of-2
    bool write_allocate;    // true if write-allocate, false if no-write-allocate
    bool write_through;     // true if write-through, false if write-back 
    bool lru;               // true if lru, false if fifo 
}; 

struct Stats {
    uint32_t total_loads; 
    uint32_t total_stores; 
    uint32_t load_hits; 
    uint32_t load_misses; 
    uint32_t store_hits; 
    uint32_t store_misses; 
    uint64_t total_cycles; 
}; 

struct Slot {
    uint32_t tag; 
    bool valid;             // cache slot contains meaninful data 
    bool dirty;             // cache slot contains newer data than memory 
    uint32_t load_ts; 
    uint32_t access_ts; 
}; 

struct Set {
    std::vector<Slot> slots; 
}; 

struct Cache {
    Config config;
    Stats stats;
    std::vector<Set> sets;
    uint32_t current_ts = 0; 

    Cache(Config my_config); 
};

bool check_params(int argc, char* argv[], Config& config);

bool is_power_of_two(uint32_t n);

uint32_t get_bits(uint32_t n); 

uint32_t get_index(const Config& config, uint32_t address); 

uint32_t get_tag(const Config& config, uint32_t address); 

void run_simulation(Cache& cache);

void access(Cache& cache, char type, uint32_t address);

int find_hit(const Set& set, uint32_t tag);

int find_evict_index(const Set& set, bool lru);

void print_stats(const Stats& stats);

#endif