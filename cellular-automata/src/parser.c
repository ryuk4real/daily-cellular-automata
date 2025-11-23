#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "parser.h"
#include "config.h"

void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Daily Cellular Automata Simulator\n\n");
    printf("Options:\n");
    printf("  -D, --daily                Generate daily random configuration (today)\n");
    printf("  -s, --seed [YYMMDD]        Use specific date seed (e.g., 251122)\n");
    printf("  -t, --test                 Run Conway's Game of Life test\n");
    printf("  -h, --help                 Show this help message\n");
}

void init_default_config(Config* config) {
    config->width = 100;
    config->height = 100;
    config->max_generations = 500;
    config->wrap_edges = 1;
    config->init_mode = INIT_RANDOM;
    config->density = 0.35f;
    config->rule_set = NULL;
    config->output_folder = strdup("output");
    config->pattern_file = NULL;
    config->seed = 0;
}

void free_config(Config* config) {
    if (config->output_folder) {
        free(config->output_folder);
        config->output_folder = NULL;
    }
    if (config->pattern_file) {
        free(config->pattern_file);
        config->pattern_file = NULL;
    }
    if (config->rule_set) {
        free(config->rule_set);
        config->rule_set = NULL;
    }
}

int parse_arguments(int argc, char** argv, Config* config) {
    init_default_config(config);
    
    // Default to daily mode if no arguments
    if (argc == 1) {
        return 2; // Daily mode
    }
    
    static struct option long_options[] = {
        {"daily",  no_argument,       0, 'D'},
        {"seed",   required_argument, 0, 's'},
        {"test",   no_argument,       0, 't'},
        {"help",   no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    int test_mode = 0;
    
    while ((c = getopt_long(argc, argv, "Ds:th", long_options, &option_index)) != -1) {
        switch (c) {
            case 'D':
                return 2; // Daily mode
            case 's':
                config->seed = (unsigned int)atoi(optarg);
                return 2; // Daily mode with specific seed
            case 't':
                test_mode = 1;
                break;
            case 'h':
                print_help(argv[0]);
                return 1;
            case '?':
                return -1;
            default:
                return -1;
        }
    }
    
    if (test_mode) {
        return 3; // Test mode (Game of Life)
    }
    
    return 2; // Default to daily mode
}
