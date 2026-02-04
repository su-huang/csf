Team Member: Su Huang, shuan148@jh.edu (Implemented all functions)

Implementation Notes
- 0 is stored as a non-negative empty vector 
- operator-() ensures that 0 is never negative 
- Made use of bitwise operations to perform multiplications by 2, isolate LSB, etc. 
- Key helper methods created: 
    - is_zero() to check if a value is 0 
    - add_magnitudes() and subtract_magnitudes() to modularize operator+()
        - Used pass by referece so carry/borrow value can be updated in these helper functions 
    - compare_magnitudes() to compare integers without considering their sign 
    - right_shift() to perform division by 2 for binary search in operator/() 