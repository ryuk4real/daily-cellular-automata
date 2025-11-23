#include <stdio.h>
#include <string.h>
#include "config.h"
#include "parser.h"
#include "rule.h"
#include "automata.h"
#include "daily.h"

void setup_game_of_life(Config* config, Rule* rule) {
    printf("> Conway's Game of Life (Test Mode)\n\n");
    
    // Conway's Game of Life: R1,C2,S2,3,B3 (Moore is default)
    rule->range = 1;
    rule->states = 2;
    rule->neighborhood = 0; // Moore
    
    memset(rule->survive, 0, sizeof(rule->survive));
    memset(rule->birth, 0, sizeof(rule->birth));
    
    rule->survive[2] = 1;
    rule->survive[3] = 1;
    rule->birth[3] = 1;
    
    config->rule_set = strdup("R1,C2,S2,3,B3");  // Moore default, no NM needed
    
    // Test configuration
    config->width = 100;
    config->height = 100;
    config->wrap_edges = 1;
    config->init_mode = INIT_RANDOM;
    config->density = 0.35f;
    config->max_generations = 200;
    
    printf("Configuration:\n");
    printf("  Rule: %s\n", config->rule_set);
    printf("  Grid: %dx%d\n", config->width, config->height);
    printf("  Wrap: Yes\n");
    printf("  Init: Random (density: %.2f)\n", config->density);
    printf("  Generations: %d\n\n", config->max_generations);
}

int main(int argc, char** argv) {
    Config config;
    Rule rule;
    
    // Parse arguments
    int parse_result = parse_arguments(argc, argv, &config);
    
    if (parse_result == 1) {
        // Help was printed
        free_config(&config);
        return 0;
    } else if (parse_result == -1) {
        // Error occurred
        free_config(&config);
        return 1;
    } else if (parse_result == 2) {
        // Daily mode
        generate_daily_config(&config, &rule);
    } else if (parse_result == 3) {
        // Test mode (Game of Life)
        setup_game_of_life(&config, &rule);
    }
    
    // Run simulation
    automata_run(&config, &rule);
    
    // Cleanup
    free_config(&config);
    return 0;
}