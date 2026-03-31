#!/bin/bash

# block size (256 sets, 4-way, write-allocate write-back lru)
./csim 256 4 4  write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256 4 8  write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256 4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256 4 32 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256 4 64 write-allocate write-back lru gcc.trace < traces/gcc.trace

./csim 256 4 4  write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256 4 8  write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256 4 16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256 4 32 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256 4 64 write-allocate write-back lru swim.trace < traces/swim.trace

# associativity (fixed ~16KB total, write-allocate write-back lru)
./csim 1024 1    16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 512  2    16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256  4    16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 128  8    16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 64   16   16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 1    1024 16 write-allocate write-back lru gcc.trace < traces/gcc.trace

./csim 1024 1    16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 512  2    16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256  4    16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 128  8    16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 64   16   16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 1    1024 16 write-allocate write-back lru swim.trace < traces/swim.trace

# total cache size (4-way, 16-byte blocks, write-allocate write-back lru)
./csim 64   4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 128  4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 256  4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 512  4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace
./csim 1024 4 16 write-allocate write-back lru gcc.trace < traces/gcc.trace

./csim 64   4 16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 128  4 16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 256  4 16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 512  4 16 write-allocate write-back lru swim.trace < traces/swim.trace
./csim 1024 4 16 write-allocate write-back lru swim.trace < traces/swim.trace

# write policy (256 sets, 4-way, 16-byte blocks, lru)
./csim 256 4 16 write-allocate   write-back  lru gcc.trace < traces/gcc.trace
./csim 256 4 16 write-allocate   write-through lru gcc.trace < traces/gcc.trace
./csim 256 4 16 no-write-allocate write-through lru gcc.trace < traces/gcc.trace

./csim 256 4 16 write-allocate   write-back  lru swim.trace < traces/swim.trace
./csim 256 4 16 write-allocate   write-through lru swim.trace < traces/swim.trace
./csim 256 4 16 no-write-allocate write-through lru swim.trace < traces/swim.trace

# eviction policy (256 sets, 4-way, 16-byte blocks, write-allocate write-back)
./csim 256 4 16 write-allocate write-back lru  gcc.trace < traces/gcc.trace
./csim 256 4 16 write-allocate write-back fifo gcc.trace < traces/gcc.trace

./csim 256 4 16 write-allocate write-back lru  swim.trace < traces/swim.trace
./csim 256 4 16 write-allocate write-back fifo swim.trace < traces/swim.trace
