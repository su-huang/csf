CONTRIBUTIONS

Su Huang, shuan148@jh.edu (Implemented all functions)

REPORT

Threshold   | Real Time | User Time | Sys Time
-----------------------------------------------
2097152     | 0.379s    | 0.364s    | 0.011s
1048576     | 0.265s    | 0.424s    | 0.038s
524288      | 0.162s    | 0.419s    | 0.044s
262144      | 0.159s    | 0.552s    | 0.058s
131072      | 0.147s    | 0.554s    | 0.076s
65536       | 0.149s    | 0.559s    | 0.103s
32768       | 0.156s    | 0.600s    | 0.110s
16384       | 0.167s    | 0.620s    | 0.178s

At the highest threshold, most of the data is sorted by a single call to qsort directly, rather than using simultaneous child processes. 
Hence, it makes sense that the highest threshold gives the slowest real time. 
 
As the threshold decreases, less data can be sorted by each child process so more recursive calls are executed in parallel by child processes. 
The OS kernel schedules these child processes across multiple CPU cores simultaneously, so more sorting can happen concurrently. 
Hence, real time drops significantly from threshold 2097152 to threshold 131072, leading to around 2.5x speedup.
 
However, below threshold 131072, real time stops improving and actually starts increasing slightly. 
At very small thresholds, the program creates a large number of child processes that all have overhead costs from forking and managing these processes. 
Eventually, the cost for each child process outweighs the benefit of parallelism. 
