#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rule.h"

static void parse_list(const char* str, uint8_t* array) {
    char* token;
    char* str_copy = strdup(str);
    char* saveptr;
    
    token = strtok_r(str_copy, ",", &saveptr);
    while (token != NULL) {
        // Check for range (e.g., "7-12")
        char* dash = strchr(token, '-');
        if (dash != NULL) {
            int start = atoi(token);
            int end = atoi(dash + 1);
            for (int i = start; i <= end && i < MAX_COUNTS; i++) {
                array[i] = 1;
            }
        } else {
            int val = atoi(token);
            if (val < MAX_COUNTS) {
                array[val] = 1;
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }
    free(str_copy);
}

int parse_rule(const char* rule_string, Rule* rule) {
    memset(rule, 0, sizeof(Rule));
    rule->neighborhood = 0; // Default Moore
    
    char* str = strdup(rule_string);
    char* token;
    char* saveptr;
    
    token = strtok_r(str, ",", &saveptr);
    while (token != NULL) {
        if (token[0] == 'R') {
            rule->range = atoi(token + 1);
        } else if (token[0] == 'C') {
            rule->states = atoi(token + 1);
        } else if (token[0] == 'S') {
            parse_list(token + 1, rule->survive);
        } else if (token[0] == 'B') {
            parse_list(token + 1, rule->birth);
        } else if (token[0] == 'N') {
            if (token[1] == 'M') {
                rule->neighborhood = 0; // Moore
            } else if (token[1] == 'N') {
                rule->neighborhood = 1; // Von Neumann
            } else {
                rule->neighborhood = atoi(token + 1);
            }
        }
        token = strtok_r(NULL, ",", &saveptr);
    }
    
    free(str);
    
    if (rule->range == 0 || rule->states == 0) {
        return -1;
    }
    return 0;
}
