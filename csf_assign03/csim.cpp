#include <iostream>
#include <string> 
#include "csim.h"

Cache::Cache(Config my_config) : config(my_config), current_ts(0) {
    stats.total_loads = 0;
    stats.total_stores = 0;
    stats.load_hits = 0;
    stats.load_misses = 0;
    stats.store_hits = 0;
    stats.store_misses = 0;
    stats.total_cycles = 0;

    sets.resize(config.num_sets); 

    for (size_t i = 0; i < config.num_sets; i++) {
        sets[i].slots.resize(config.num_blocks); 

        for (size_t j = 0; j < config.num_blocks; j++) {
            sets[i].slots[j].tag = 0; 
            sets[i].slots[j].valid = false; 
            sets[i].slots[j].dirty = false; 
            sets[i].slots[j].load_ts = 0; 
            sets[i].slots[j].access_ts = 0; 
        }
    }
}

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
    if (write_alloc == "write-allocate") {
        config.write_allocate = true; 
    } else if (write_alloc == "no-write-allocate") {
        config.write_allocate = false; 
    } else {
        std::cerr << "Error: Expected 'write-allocate' or 'no-write-allocate'" << std::endl; 
        return false; 
    }

    std::string write_through = argv[5]; 
    if (write_through == "write-through") {
        config.write_through = true; 
    } else if (write_through == "write-back") {
        config.write_through = false; 
    } else {
        std::cerr << "Error: Expected 'write-through' or 'write-back'" << std::endl; 
        return false; 
    }

    if (!config.write_allocate && !config.write_through) {
        std::cerr << "Error: Doesn’t make sense to combine no-write-allocate with write-back" << std::endl; 
        return false; 
    }

    std::string evict_type = argv[6]; 
    if (evict_type == "lru") {
        config.lru = true; 
    } else if (evict_type == "fifo") {
        config.lru = false;
    } else {
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

void run_simulation(Cache& cache) {
    char type; 
    uint32_t address; 
    int garbage; 

    while (std::cin >> type >> std::hex >> address >> std::dec >> garbage) {
        access(cache, type, address); 
    }
}

void access(Cache& cache, char type, uint32_t address) {
    cache.current_ts++; 
    uint32_t index = get_index(cache.config, address); 
    uint32_t tag = get_tag(cache.config, address); 

    bool store = (type == 's'); 
    if (store) {
        cache.stats.total_stores++; 
    } else {
        cache.stats.total_loads++; 
    } 

    int hit_index = find_hit(cache.sets[index], tag); 
    cache.stats.total_cycles += 1;

    if (hit_index != -1) {
        cache.sets[index].slots[hit_index].access_ts = cache.current_ts;

        if (store) {
            cache.stats.store_hits++; 
            if (cache.config.write_through) {
                cache.stats.total_cycles += 100;
            } else {
                cache.sets[index].slots[hit_index].dirty = true; 
            } 
        } else {
            cache.stats.load_hits++; 
        }
    } else {
        // miss 
        if (store) {
            cache.stats.store_misses++;
        } else {
            cache.stats.load_misses++; 
        }

        // write block to cache from memory 
        if (!store || cache.config.write_allocate) {
            cache.stats.total_cycles += 100 * (cache.config.num_bytes / 4); 

            // find a slot 
            int slot_index = -1; 
            for (size_t i = 0; i < cache.sets[index].slots.size(); i++) {
                if (!cache.sets[index].slots[i].valid) {
                    slot_index = static_cast<int>(i); 
                    break; 
                }
            }

            // need to evict a slot if set full 
            if (slot_index == -1) {
                slot_index = find_evict_index(cache.sets[index], cache.config.lru);

                // write-back: dirty block must be written back to memory 
                if (!cache.config.write_through && cache.sets[index].slots[slot_index].dirty) {
                    cache.stats.total_cycles += 100 * (cache.config.num_bytes / 4);
                }
            }

            // update slot 
            Slot& new_slot = cache.sets[index].slots[slot_index];
            new_slot.valid = true;
            new_slot.tag = tag;
            new_slot.load_ts = cache.current_ts;   
            new_slot.access_ts = cache.current_ts; 

            if (store) {
                if (cache.config.write_through) {
                    // write to memory immediately so cache slot and memory are synced 
                    cache.stats.total_cycles += 100;
                    new_slot.dirty = false; 
                } else {
                    // don't write to memory so cache slot is ahead of memory 
                    new_slot.dirty = true; 
                }
            } else {
                // load from memory so cache slot and memory are synced 
                new_slot.dirty = false; 
            }
        } else {
            // store and no-write-allocate: don't change cache, just write to memory 
            cache.stats.total_cycles += 100; 
        }
    }
}

int find_hit(const Set& set, uint32_t tag) {
    for (size_t i = 0; i < set.slots.size(); i++) {
        if (set.slots[i].valid && set.slots[i].tag == tag) return static_cast<int>(i); 
    }
    return -1; 
}

int find_evict_index(const Set& set, bool lru) {
    int ans_index = 0; 
    uint32_t min_ts = lru ? set.slots[0].access_ts : set.slots[0].load_ts; 

    for (size_t i = 1; i < set.slots.size(); i++) {
        uint32_t curr_ts = lru ? set.slots[i].access_ts : set.slots[i].load_ts; 
        if (curr_ts < min_ts) {
            ans_index = static_cast<int>(i); 
            min_ts = curr_ts; 
        }
    }

    return ans_index; 
}

void print_stats(const Stats& stats) {
    std::cout << "Total loads: " << stats.total_loads << std::endl;
    std::cout << "Total stores: " << stats.total_stores << std::endl;
    std::cout << "Load hits: " << stats.load_hits << std::endl;
    std::cout << "Load misses: " << stats.load_misses << std::endl;
    std::cout << "Store hits: " << stats.store_hits << std::endl;
    std::cout << "Store misses: " << stats.store_misses << std::endl;
    std::cout << "Total cycles: " << stats.total_cycles << std::endl;
}