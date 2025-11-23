#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "grid.h"

Grid* grid_create(int width, int height) {
    Grid* grid = malloc(sizeof(Grid));
    grid->width = width;
    grid->height = height;
    grid->cells = calloc(width * height, sizeof(uint8_t));
    return grid;
}

void grid_free(Grid* grid) {
    if (grid) {
        free(grid->cells);
        free(grid);
    }
}

void grid_init_random(Grid* grid, float density) {
    int alive_count = 0;
    for (int i = 0; i < grid->width * grid->height; i++) {
        if (rand() / (float)RAND_MAX < density) {
            grid->cells[i] = 1;
            alive_count++;
        } else {
            grid->cells[i] = 0;
        }
    }
    printf("Initialized grid with %d alive cells (%.2f%% density)\n", 
           alive_count, (100.0f * alive_count) / (grid->width * grid->height));
}

void grid_init_center(Grid* grid) {
    memset(grid->cells, 0, grid->width * grid->height);
    int cx = grid->width / 2;
    int cy = grid->height / 2;
    
    // Create a small cluster in the center instead of just one cell
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            if (abs(dx) + abs(dy) <= 3) {
                int x = cx + dx;
                int y = cy + dy;
                if (x >= 0 && x < grid->width && y >= 0 && y < grid->height) {
                    grid->cells[y * grid->width + x] = 1;
                }
            }
        }
    }
    printf("Initialized grid with center cluster\n");
}

int grid_save(const Grid* grid, const char* folder, int generation) {
    // Create output folder
    mkdir(folder, 0755);
    
    char filename[512];
    snprintf(filename, sizeof(filename), "%s/gen_%06d.bin", folder, generation);
    
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return -1;
    }
    
    // Write dimensions first, then data
    fwrite(&grid->width, sizeof(int), 1, file);
    fwrite(&grid->height, sizeof(int), 1, file);
    fwrite(grid->cells, sizeof(uint8_t), grid->width * grid->height, file);
    
    fclose(file);
    return 0;
}
