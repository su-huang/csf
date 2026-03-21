/*
 * Main implementation for cache simulation 
 * CSF Assignment 3 
 * Su Huang 
 * shuan148@jh.edu
 */

#include <iostream>
#include "csim.h"

int main( int argc, char **argv ) {
	Config config; 
	if (!check_params(argc, argv, config)) {
        return 1;
    }

	Cache cache(config); 
	
	run_simulation(cache); 
	print_stats(cache.stats); 

	return 0;
}
