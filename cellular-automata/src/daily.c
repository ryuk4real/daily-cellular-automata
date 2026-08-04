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

static int compare_ints(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

static void generate_conditions(uint8_t* array, int max_neighbors) {
    int max_points = max_neighbors / 5;
    if (max_points < 3) max_points = 3;
    if (max_points > 15) max_points = 15;

    int num_points = random_range(2, max_points);
    if (num_points > max_neighbors + 1) num_points = max_neighbors + 1;

    int* pool = malloc((max_neighbors + 1) * sizeof(int));
    for (int i = 0; i <= max_neighbors; i++) {
        pool[i] = i;
    }

    for (int i = 0; i < num_points; i++) {
        int j = i + (rand() % (max_neighbors + 1 - i));
        int tmp = pool[i];
        pool[i] = pool[j];
        pool[j] = tmp;
    }

    int* points = malloc(num_points * sizeof(int));
    memcpy(points, pool, num_points * sizeof(int));
    free(pool);

    qsort(points, num_points, sizeof(int), compare_ints);

    int i = 0;
    while (i < num_points) {
        if (i + 1 < num_points && random_range(0, 1)) {
            for (int j = points[i]; j <= points[i + 1]; j++) {
                array[j] = 1;
            }
            i += 2;
        } else {
            array[points[i]] = 1;
            i++;
        }
    }

    free(points);
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
    rule->neighborhood = random_range(0, 2);
    
    // Calculate max_neighbors
    int max_neighbors = 0;
    if (rule->neighborhood == 0) {
        max_neighbors = (2 * rule->range + 1) * (2 * rule->range + 1) - 1;
    } else if (rule->neighborhood == 1) {
        max_neighbors = 2 * rule->range * (rule->range + 1);
    } else if (rule->neighborhood == 2) {
        int r_sq = rule->range * rule->range;
        for (int dy = -rule->range; dy <= rule->range; dy++) {
            for (int dx = -rule->range; dx <= rule->range; dx++) {
                if (dx == 0 && dy == 0) continue;
                if (dx * dx + dy * dy <= r_sq) {
                    max_neighbors++;
                }
            }
        }
    }
    
    memset(rule->survive, 0, sizeof(rule->survive));
    memset(rule->birth, 0, sizeof(rule->birth));

    generate_conditions(rule->survive, max_neighbors);
    generate_conditions(rule->birth, max_neighbors);
    
    char rule_str[4096];
    int pos = snprintf(rule_str, sizeof(rule_str), "R%d,C%d,S", rule->range, rule->states);

    int first = 1;
    for (int i = 0; i <= max_neighbors && pos < 4000; i++) {
        if (rule->survive[i]) {
            if (!first) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",");
            }
            
            int start = i;
            int end = i;
            while (end + 1 <= max_neighbors && rule->survive[end + 1]) {
                end++;
            }

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

    first = 1;
    for (int i = 0; i <= max_neighbors && pos < 4000; i++) {
        if (rule->birth[i]) {
            if (!first) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",");
            }
            
            int start = i;
            int end = i;
            while (end + 1 <= max_neighbors && rule->birth[end + 1]) {
                end++;
            }

            if (end > start) {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d-%d", start, end);
            } else {
                pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, "%d", start);
            }
            
            i = end;
            first = 0;
        }
    }

    if (rule->neighborhood == 1) {
        pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",NN");
    } else if (rule->neighborhood == 2) {
        pos += snprintf(rule_str + pos, sizeof(rule_str) - pos, ",NC");
    }

    config->rule_set = strdup(rule_str);

    config->wrap_edges = 1;
    config->init_mode = INIT_RANDOM;
    config->density = 0.35f;
    config->max_generations = 200;
    
    const char* nbr_name = "Moore";
    if (rule->neighborhood == 1) nbr_name = "Von Neumann";
    else if (rule->neighborhood == 2) nbr_name = "Circular";

    printf("\n> Generated Daily Rule\n");
    printf("Rule: %s\n\n", config->rule_set);
    printf("Range: %d, States: %d, Neighborhood: %s\n", 
           rule->range, rule->states, nbr_name);
    printf("Grid: %dx%d, Generations: %d\n\n", 
           config->width, config->height, config->max_generations);

    mkdir(config->output_folder, 0755);
    
    char info_path[512];
    snprintf(info_path, sizeof(info_path), "%s/rule_info.txt", config->output_folder);
    
    FILE* info_file = fopen(info_path, "w");
    if (info_file) {
        fprintf(info_file, "Seed: %u\n", seed);
        fprintf(info_file, "Rule: %s\n", config->rule_set);
        fprintf(info_file, "Generations: %d\n", config->max_generations);
        fprintf(info_file, "Neighborhood: %s\n", nbr_name);
        fclose(info_file);
    }
}
