Team Member: Su Huang, shuan148@jh.edu (Implemented all functions)

C Implementation Notes
- Used all proposed helper methods including PixelAverager 
- Divided expansion helper methods based on even/odd row/col indices to improve readability of main function 
- Makefile provides an error because the helpder methods aren't defined for the assembly tests, but that doesn't impact the workings of the C program 
- Wrote modularized unit tests for each helper function, making sure to verify edge cases 

Assembly Implementation Notes 
- Implemented all helper functions identical to those written in C
- Didn't write any new helpfer function tests because the C tests cover the functionality of all assembly functions 
- Followed x86 register use conventions for stack registers, argument registers, etc. 
- Header comments indicating callee-saved registers and memory use 
- Used local labels for control flow 
