#ifndef GRID_H
#define GRID_H

#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t* cells;
    int width;
    int height;
} Grid;

// Initialize grid
Grid* grid_create(int width, int height);
void grid_free(Grid* grid);

// Initialize patterns
void grid_init_random(Grid* grid, float density);
void grid_init_center(Grid* grid);

// File I/O
int grid_save(const Grid* grid, const char* folder, int generation);

#endif // GRID_H
