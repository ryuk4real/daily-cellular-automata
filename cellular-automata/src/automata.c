#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "automata.h"

static inline int count_neighbors(const Grid* grid, int x, int y, int range, int wrap, int neighborhood) {
    int count = 0;
    int w = grid->width;
    int h = grid->height;
    
    for (int dy = -range; dy <= range; dy++) {
        for (int dx = -range; dx <= range; dx++) {
            if (dx == 0 && dy == 0) continue;
            
            // Von Neumann neighborhood: Manhattan distance <= range
            if (neighborhood == 1) {
                if (abs(dx) + abs(dy) > range) continue;  // This creates rhombus shape
            }
            
            int nx = x + dx;
            int ny = y + dy;
            
            if (wrap) {
                nx = ((nx % w) + w) % w;
                ny = ((ny % h) + h) % h;
            } else {
                if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
            }
            
            // Only State 1 cells count as neighbors
            // State 0 = dead (0), State 1 = alive (1), State 2+ = dying/obstacles (0)
            if (grid->cells[ny * w + nx] == 1) {
                count++;
            }
        }
    }
    
    return count;
}

int automata_step(const Grid* current, Grid* next, const Rule* rule, int wrap) {
    int w = current->width;
    int h = current->height;
    int changed_count = 0;
    
    #pragma omp parallel for collapse(2) schedule(dynamic) reduction(+:changed_count)
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            uint8_t cell = current->cells[idx];
            uint8_t next_state = 0;
            
            if (cell == 0) {
                // Dead cell (State 0): check birth
                int neighbors = count_neighbors(current, x, y, rule->range, wrap, rule->neighborhood);
                next_state = rule->birth[neighbors] ? 1 : 0;
            } else if (cell == 1) {
                // Alive cell (State 1): check survival
                int neighbors = count_neighbors(current, x, y, rule->range, wrap, rule->neighborhood);
                
                if (rule->survive[neighbors]) {
                    next_state = 1;  // Stays alive
                } else {
                    // Dies: goes to State 2 if multi-state, otherwise State 0
                    next_state = (rule->states > 2) ? 2 : 0;
                }
            } else {
                // Dying cell (State 2+): automatically increment, no neighbor check
                if (cell + 1 < rule->states) {
                    next_state = cell + 1;  // Continue dying
                } else {
                    next_state = 0;  // Finally dead
                }
            }
            
            next->cells[idx] = next_state;
            if (cell != next_state) {
                changed_count++;
            }
        }
    }
    
    return changed_count;
}

void automata_run(const Config* config, const Rule* rule) {
    Grid* current = grid_create(config->width, config->height);
    Grid* next = grid_create(config->width, config->height);
    
    // Initialize
    switch (config->init_mode) {
        case INIT_RANDOM:
            // Only set new random seed if not already set (daily mode sets it)
            if (config->seed == 0) {
                srand(time(NULL) ^ clock());
            }
            // If seed is already set (daily mode), srand() was already called
            grid_init_random(current, config->density);
            break;
        case INIT_CENTER:
            grid_init_center(current);
            break;
        case INIT_FILE:
            fprintf(stderr, "File initialization not yet implemented\n");
            if (config->seed == 0) {
                srand(time(NULL) ^ clock());
            }
            grid_init_random(current, config->density);
            break;
    }
    
    printf("Starting simulation...\n");
    printf("Threads: %d\n", omp_get_max_threads());
    
    // Count initial alive cells
    int initial_alive = 0;
    for (int i = 0; i < current->width * current->height; i++) {
        if (current->cells[i] > 0) initial_alive++;
    }
    printf("Initial alive cells: %d\n\n", initial_alive);
    
    // Start timing
    double start_time = omp_get_wtime();
    
    int generation = 0;
    int max_gen = config->max_generations;
    long long total_changed_cells = 0;
    
    while (max_gen == 0 || generation < max_gen) {
        // Save current generation
        if (grid_save(current, config->output_folder, generation) != 0) {
            fprintf(stderr, "Failed to save generation %d\n", generation);
            break;
        }
        
        // Count alive cells for progress
        int alive = 0;
        for (int i = 0; i < current->width * current->height; i++) {
            if (current->cells[i] > 0) alive++;
        }
        
        if (generation % 100 == 0) {
            printf("Generation %d - Alive cells: %d (%.2f%%)\n", 
                   generation, alive, (100.0f * alive) / (current->width * current->height));
        }
        
        // Compute next generation
        int changed = automata_step(current, next, rule, config->wrap_edges);
        total_changed_cells += changed;
        
        // Swap buffers
        Grid* temp = current;
        current = next;
        next = temp;
        
        generation++;
    }
    
    // End timing
    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;
    
    // Calculate Activity Score
    float activity_score = 0.0f;
    if (generation > 0) {
        activity_score = (float)total_changed_cells / (generation * (long long)config->width * config->height) * 100.0f;
    }
    
    printf("\nSimulation complete. %d generations saved.\n", generation);
    printf("Execution time: %.2f seconds (%.2f ms per generation)\n", 
           elapsed_time, (elapsed_time * 1000.0) / generation);
    printf("Activity Score: %.2f%%\n", activity_score);
    
    // Append score to rule_info.txt
    char info_path[512];
    snprintf(info_path, sizeof(info_path), "%s/rule_info.txt", config->output_folder);
    FILE* info_file = fopen(info_path, "a");
    if (info_file) {
        fprintf(info_file, "Activity: %.2f%%\n", activity_score);
        fclose(info_file);
    }
    
    grid_free(current);
    grid_free(next);
}
