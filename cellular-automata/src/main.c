#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
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

    mkdir(config->output_folder, 0755);
    char info_path[512];
    snprintf(info_path, sizeof(info_path), "%s/rule_info.txt", config->output_folder);
    FILE* info_file = fopen(info_path, "w");
    if (info_file) {
        fprintf(info_file, "Rule: %s\n", config->rule_set);
        fprintf(info_file, "Generations: %d\n", config->max_generations);
        fprintf(info_file, "Neighborhood: Moore\n");
        fclose(info_file);
    }
}

void setup_circular_test(Config* config, Rule* rule) {
    printf("> Circular Neighborhood Test Mode (NC)\n\n");
    
    // Simple Circular rule: R2,C2,S3-5,B3,NC
    if (parse_rule("R2,C2,S3-5,B3,NC", rule) != 0) {
        fprintf(stderr, "Error parsing circular test rule\n");
        return;
    }
    
    config->rule_set = strdup("R2,C2,S3-5,B3,NC");
    
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

    mkdir(config->output_folder, 0755);
    char info_path[512];
    snprintf(info_path, sizeof(info_path), "%s/rule_info.txt", config->output_folder);
    FILE* info_file = fopen(info_path, "w");
    if (info_file) {
        fprintf(info_file, "Rule: %s\n", config->rule_set);
        fprintf(info_file, "Generations: %d\n", config->max_generations);
        fprintf(info_file, "Neighborhood: Circular\n");
        fclose(info_file);
    }
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
    } else if (parse_result == 4) {
        // Test mode (Circular NC)
        setup_circular_test(&config, &rule);
    }
    
    // Run simulation
    automata_run(&config, &rule);
    
    // Cleanup
    free_config(&config);
    return 0;
}