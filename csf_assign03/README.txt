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
    - FIFO: uses 'load_ts', which is only set when the block is written to the cache

---
Best Cache Report 
- Ran 5 experiments across the two provided real-world traces, gcc.trace and swim.trace
- Primary metric: total cycles -> illustrates the combined effect of hit rate, miss penalty, and write policy cost in a single number

1. Block Size 
- Configuration: 256 sets, 4-way, write-allocate, write-back, lru
- Variable block sizes: 4, 8, 16, 32, 64 bytes 
- Goal: evaluate the tradeoff between spatial locality and miss penalty 

Results (total cycles):
  Block size | gcc.trace  | swim.trace
  -----------+------------+-----------
  4B         | 7,187,283  | 5,304,493
  8B         | 9,015,083  | 8,156,593
  16B        | 9,344,483  | 9,009,593
  32B        | 9,543,683  | 9,602,393
  64B        | 9,707,683  | 9,591,193

Observations:
Total cycles increases with block size beyond 16B, as the higher miss penalty outweighs the hit rate improvement.
While larger blocks do improve hit rate, the cost of transferring more 4-byte chunks from memory on each miss grows proportionally.
4B has the lowest raw cycle count but is impractical because a 4-byte block holds exactly one memory access and there is significant metadata overhead. 
I think that 16B blocks provide the best practical tradeoff because while cycle counts are higher than 4B, each block can now hold 4x 4-byte accesses, providing meaningful spatial locality.
The metadata overhead is quartered relative to 4B and beyond 18B, each doubling of block size gives decreasing hit rate gains while the miss penalty continues to grow. 

2. Associativity 
- Configuration: 16KB total cache, 16B blocks, write-allocate, write-back, lru
- Variable associativity: 1, 2, 4, 8, 16, 1024-way and sets adjusted to keep total size constant 
- Goal: evaluate the tradeoff between conflict misses and hardware complexity 

Results (total cycles):
  Associativity | gcc.trace   | swim.trace
  --------------+-------------+-----------
  1-way         | 11,127,283  | 10,171,593
  2-way         |  9,588,883  |  9,099,193
  4-way         |  9,344,483  |  9,009,593
  8-way         |  9,277,683  |  8,991,593
  16-way        |  9,266,883  |  8,976,793
  1024-way      |  9,226,883  |  8,965,993

Observations: 
The largest improvement in total cycles is from 1-way to 2-way. 
After 4-way, the benefit of additonal associativity decreases rapidly while hardware csots rise significantly, so I thinkg 4-way associativity is optimal. 

3. Cache size 
- Configuration: 4-way, 16B blocks, write-allocate, write-back, lru
- Variable cache size: 4, 8, 16, 32, 64 KB achieved by modifying the number of sets 
- Goal: determine when increasing cache size leads to diminishing returns

Results (total cycles):
  Cache size | gcc.trace   | swim.trace
  -----------+-------------+-----------
  4KB        | 11,414,883  | 11,733,193
  8KB        | 10,071,683  |  9,549,993
  16KB       |  9,344,483  |  9,009,593
  32KB       |  8,607,683  |  8,521,593
  64KB       |  7,620,883  |  7,744,393

Observations: 
Cycles decrease steadily as cache size grows, but the contextual gains shrink. 
Going from 4KB to 16KB saves ~2M cycles but quadrupling the cache size from 16KB to 64KB saves only 1.7M cycles. 
Hence, 6KB represents a reasonable balance between performance and hardware cost.

4. Write policy 
- Configuration: 256 sets, 4-way, 16B blocks, lru
- Variable write policy combinations: write-allocate/write-back, write-allocate/write-through, no-write-allocate/write-through 
- Goal: evluate and compare the total cycle count for the 3 write policy combiantions 

Results (total cycles):
  Policy          | gcc.trace   | swim.trace
  ----------------+-------------+-----------
  wa / write-back |  9,344,483  |  9,009,593
  wa / write-thru | 25,318,283  | 13,247,693
  nwa / write-thru| 22,897,883  |  9,594,093

Observations: 
Write-through is more costly in general, especially on gcc.trace, costly 2.7x more cycles than write-back for gcc because every store hit requires a memory write.
Write-allocate with write-back is the clear winner with the minimum number of total cycles for both traces. 
No-write-allocate with write-through performs better on swim.trace because it likely has fewer stores relative to loads, reducing the write-through penalty.

5. Eviction policy 
- Configuration: 256 sets, 4-way, 16B blocks, write-allocate, write-back
- Variable eviction policy: lru, fifo

Results (total cycles):
  Policy | gcc.trace  | swim.trace
  -------+------------+-----------
  lru    | 9,344,483  | 9,009,593
  fifo   | 9,845,283  | 9,655,593

Observations: 
- LRU outperforms FIFO on both traces with a small but consistent difference. 
- LRU's load-based eviction better captures temporal locality in these traces.

---
Best overall configuration: 256 sets, 4-way, 16B blocks, write-allocate, write-back, lru

- Block size 16B: avoids the high miss penalty of larger blocks while still benefiting from spatial locality
- 4-way associativity: experiences most of the benefit of full associativity with far less hardware overhead
- 16KB total size: good performance without excessive hardware cost
- Write-allocate + write-back: dramatically fewer memory writes than write-through
- LRU eviction: consistently outperforms FIFO across both traces

This configuration achieves 9,344,483 cycles on gcc.trace and 9,009,593 cycles on swim.trace, with hit rates of 97.6% and 96.1% respectively.
Graphs demonstrating the results can be found here: https://github.com/su-huang/csf/blob/main/csf_assign03/experiments/cache_results.png 
