/*
 * Main implementation for cache simulation 
 * CSF Assignment 3 
 * Su Huang 
 * shuan148@jh.edu
 */

#include <iostream>
#include "csim.h"

//! Handles command-line arguments and simulation lifecycle
//! @param argc number of command line arguments
//! @param argv array of command line argument strings
int main(int argc, char **argv) {
	Config config; 

	// validate parameters (errors printed in check_params())
	if (!check_params(argc, argv, config)) {
        return 1;
    }

	Cache cache(config); 
	
	run_simulation(cache); 
	print_stats(cache.stats); 

	return 0;
}
