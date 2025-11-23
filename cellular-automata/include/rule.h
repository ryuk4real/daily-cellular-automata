#ifndef RULE_H
#define RULE_H

#include <stdint.h>

// For R10 Moore: (2*10+1)^2 - 1 = 440
#define MAX_COUNTS 500  // Sufficient for R10

typedef struct {
    int range;              // R value
    int states;             // C value
    uint8_t survive[MAX_COUNTS];  // Survival counts (1 = survives)
    uint8_t birth[MAX_COUNTS];    // Birth counts (1 = births)
    int neighborhood;       // N value (0=Moore, 1=von Neumann, 2=Hexagonal)
} Rule;

// Parse HROT notation rule string
int parse_rule(const char* rule_string, Rule* rule);

#endif // RULE_H
