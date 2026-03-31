/*
 * Header for cache simulation and helper functions 
 * CSF Assignment 3
 * Su Huang 
 * shuan148@jh.edu
 */

#ifndef CSIM_H
#define CSIM_H

#include <stdint.h>
#include <vector>
#include <string>

//! User-defined configuration settings for the cache simulation
struct Config {
    uint32_t num_sets;      // positive power-of-2
    uint32_t num_blocks;    // positive power-of-2
    uint32_t num_bytes;     // positive power-of-2
    bool write_allocate;    // true if write-allocate, false if no-write-allocate
    bool write_through;     // true if write-through, false if write-back 
    bool lru;               // true if lru, false if fifo 
}; 

//! Statistics tracked during the simulation
struct Stats {
    uint32_t total_loads;   
    uint32_t total_stores; 
    uint32_t load_hits; 
    uint32_t load_misses; 
    uint32_t store_hits; 
    uint32_t store_misses; 
    uint64_t total_cycles; 
}; 

//! Represents a single block in a cache set
struct Slot {
    uint32_t tag; 
    bool valid;             // cache slot contains meaninful data 
    bool dirty;             // cache slot contains newer data than memory 
    uint32_t load_ts; 
    uint32_t access_ts; 
}; 

//! Represents a set containing one or more slots
struct Set {
    std::vector<Slot> slots; 
}; 

//! Over-arching cache structure 
struct Cache {
    Config config;
    Stats stats;
    std::vector<Set> sets;
    uint32_t current_ts = 0;    // global timestamp 

    //! Constructor to initialize the cache based on user-defined configurations 
    //! @param my_config configuration parameters set to match user input 
    Cache(Config my_config); 
};

//! Validates command line arguments and assigns the config struct accordingly 
//! Prints error message to std::cerr
//! @param argc number of command line arguments
//! @param argv array of command line argument strings
//! @param config reference to the config struct
//! @return true if all parameters are valid, false otherwise
bool check_params(int argc, char* argv[], Config& config);

//! Checks if a number is a power of two
//! @param n number to check
//! @return true if n is a power of two, false otherwise
bool is_power_of_two(uint32_t n);

//! Calculates the number of bits needed to represent a number of bytes (effectively calculating log2)
//! @param n number of bytes to calculate bits for
//! @return number of bits
uint32_t get_bits(uint32_t n); 

//! Isolates the set index bits from a memory address
//! @param config reference to the config struct
//! @param address 32-bit memory address
//! @return index of the set
uint32_t get_index(const Config& config, uint32_t address); 

//! Isolates the tag bits from a memory address
//! @param config reference to the config struct
//! @param address 32-bit memory address
//! @return tag for the address
uint32_t get_tag(const Config& config, uint32_t address); 

//! Main loop that reads and executes each line from the trace file 
//! @param cache reference to the cache struct 
void run_simulation(Cache& cache);

//! Executes a single memory access (load or store)
//! @param cache reference to the cache struct 
//! @param type 'l' for load, 's' for store
//! @param address 32-bit memory address
void access(Cache& cache, char type, uint32_t address);

//! Searches for the given tag within a specified set
//! @param set reference to the set struct
//! @param tag tag to look for
//! @return index of the slot with a matching tag if found (hit), -1 otherwise (miss)
int find_hit(const Set& set, uint32_t tag);

//! Determines which slot to evict based on LRU or FIFO policy
//! @param set reference to the set struct
//! @param lru true for LRU, false for FIFO
//! @return index of the slot to be evicted
int find_evict_index(const Set& set, bool lru);

//! Prints the final simulation statistics to standard output
//! @param stats reference to the stats struct 
void print_stats(const Stats& stats);

//! Appends simulation results and parameters to a CSV file
//! @param stats reference to the stats struct
//! @param config reference to the config struct
//! @param trace_file name of the trace file used
//! @param filename name of the CSV file to append to
void append_stats_csv(const Stats& stats, const Config& config, const std::string& trace_file, const std::string& filename = "results.csv"); 

#endif