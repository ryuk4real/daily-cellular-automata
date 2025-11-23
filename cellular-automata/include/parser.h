#ifndef PARSER_H
#define PARSER_H

#include "config.h"

// Parse command line arguments and populate config
int parse_arguments(int argc, char** argv, Config* config);

// Free allocated memory in config
void free_config(Config* config);

// Print help message
void print_help(const char* program_name);

// Parse JSON config file
int parse_config_file(const char* filename, Config* config);

// Add seed parameter to parser
void add_seed_parameter(Config* config, int seed);

#endif // PARSER_H
