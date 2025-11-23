#ifndef AUTOMATA_H
#define AUTOMATA_H

#include "grid.h"
#include "rule.h"
#include "config.h"

// Run one generation step and return number of changed cells
int automata_step(const Grid* current, Grid* next, const Rule* rule, int wrap);

// Run simulation
void automata_run(const Config* config, const Rule* rule);

#endif // AUTOMATA_H
