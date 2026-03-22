Team Member: Su Huang, shuan148@jh.edu (Implemented all functions)

Implementation Notes
- Used uint64_t for total_cycles to prevent 32-bit overflow 
- No-write-allocate is incompatible with write-back
- Cycle counting: 
    - Every access starts with a 1-cycle cache hit/miss check 
    - Fetching block on miss or evicting dirty blocks cost 100 cycles for every 4-byte word in the block
    - Write-through or non-allocated stores update only a single 4-byte word in memory, costing 100 cycles
- Eviction: 
    - LRU: uses access_ts, which is updated on every hit and miss
    - FIFO: uses 'load_ts', which is only set when the block is written to the cache.

TODO (for MS3): best cache report
