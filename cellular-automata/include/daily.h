#ifndef DAILY_H
#define DAILY_H

#include "config.h"
#include "rule.h"

// Generate daily configuration based on current date
void generate_daily_config(Config* config, Rule* rule);

// Get seed from current date (YYYYMMDD format)
unsigned int get_daily_seed(void);

#endif // DAILY_H
