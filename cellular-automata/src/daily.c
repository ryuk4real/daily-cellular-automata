#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "daily.h"

unsigned int get_daily_seed(void) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    // Format: YYYYMMDD (e.g., 20251122 for Nov 22, 2025)
    unsigned int seed = (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
    return seed;
}

static int random_range(int min, int max) {
    return min + (rand() % (max - min + 1));
}

void generate_daily_config(Config* config, Rule* rule) {
    unsigned int seed;
    
    if (config->seed != 0) {
        seed = config->seed;
        printf("> Daily Cellular Automata\n");
        printf("Date seed: %u (custom)\n", seed);
    } else {
        seed = get_daily_seed();
        printf("> Daily Cellular Automata\n");
        printf("Date seed: %u (today)\n", seed);
    }
    
    srand(seed);
    
    // Fixed grid parameters
    config->width = 100;
    config->height = 100;
    
    // Generate random rule parameters
    rule->range = random_range(1, 8);
    rule->states = random_range(2, 16);
    rule->neighborhood = random_range(0, 1); // TODO: Add more type of neighbourhoods
    
    // Calculate max_neighbors
    int max_neighbors;
    if (rule->neighborhood == 0) {
        max_neighbors = (2 * rule->range + 1) * (2 * rule->range + 1) - 1;
    } else {
        max_neighbors = 2 * rule->range * (rule->range + 1);
    }
    
    memset(rule->survive, 0, sizeof(rule->survive));
    memset(rule->birth, 0, sizeof(rule->birth));
    
    // Generate rules
    for (int i = 0; i <= max_neighbors; i++) {
        if (random_range(1, 100) <= 15) {
            rule->survive[i] = 1;
        }
        if (random_range(1, 100) <= 15) {
            rule->birth[i] = 1;
        }
    }
    
    // Build rule string - use large fixed buffer
    char rule_str[4096];  // Large fixed buffer
    int pos = snprintf(rule_str, sizeof(rule_str), "R%d,C%d,S", rule->range, rule->states);
    
    // Write survival rules
    int first = 1;
    for (int i = 0; i <= max_neighbors && pos < 4000; i++) {
        if (rule->survive[i]) {
            if (!first) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",");
            }
            
            int start = i;
            int end = i;
            
            // Find consecutive values
            while (end + 1 <= max_neighbors && rule->survive[end + 1]) {
                end++;
            }
            
            // Write range or single value
            if (end > start) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d-%d", start, end);
            } else {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d", start);
            }
            
            i = end;
            first = 0;
        }
    }
    
    pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",B");
    
    // Write birth rules
    first = 1;
    for (int i = 0; i <= max_neighbors && pos < 4000; i++) {
        if (rule->birth[i]) {
            if (!first) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",");
            }
            
            int start = i;
            int end = i;
            
            // Find consecutive values
            while (end + 1 <= max_neighbors && rule->birth[end + 1]) {
                end++;
            }
            
            // Write range or single value
            if (end > start) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d-%d", start, end);
            } else {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d", start);
            }
            
            i = end;
            first = 0;
        }
    }
    
    // Add neighborhood
    if (rule->neighborhood == 1) {
        pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",NN");
    }
    
    // Duplicate string for config
    config->rule_set = strdup(rule_str);
    
    // Fixed parameters
    config->wrap_edges = 1;
    config->init_mode = INIT_RANDOM;
    config->density = 0.35f;
    config->max_generations = 200;
    
    printf("\n> Generated Daily Rule\n");
    printf("Rule: %s\n\n", config->rule_set);
    printf("Range: %d, States: %d, Neighborhood: %s\n", 
           rule->range, rule->states, 
           rule->neighborhood == 0 ? "Moore" : "Von Neumann");
    printf("Grid: %dx%d, Generations: %d\n\n", 
           config->width, config->height, config->max_generations);
    
    // Save rule info to file for Telegram bot
    // Ensure output folder exists
    mkdir(config->output_folder, 0755);
    
    char info_path[512];
    snprintf(info_path, sizeof(info_path), "%s/rule_info.txt", config->output_folder);
    
    FILE* info_file = fopen(info_path, "w");
    if (info_file) {
        fprintf(info_file, "Seed: %u\n", seed);
        fprintf(info_file, "Rule: %s\n", config->rule_set);
        fprintf(info_file, "Generations: %d\n", config->max_generations);
        fprintf(info_file, "Neighborhood: %s\n", 
                rule->neighborhood == 0 ? "Moore" : "Von Neumann");
        fclose(info_file);
    }
}
