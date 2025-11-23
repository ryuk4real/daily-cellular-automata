#ifndef CONFIG_H
#define CONFIG_H

#define MAX_RULE_LENGTH 256

typedef enum {
    INIT_RANDOM,
    INIT_CENTER,
    INIT_FILE
} InitMode;

typedef struct {
    int width;
    int height;
    int max_generations;
    char* output_folder;
    int wrap_edges;
    InitMode init_mode;
    float density;
    unsigned int seed;
    char* pattern_file;
    char* rule_set;
} Config;

// Default values
#define DEFAULT_WIDTH 100
#define DEFAULT_HEIGHT 100
#define DEFAULT_GENERATIONS 0
#define DEFAULT_DENSITY 0.5f
#define DEFAULT_OUTPUT_FOLDER "output"

#endif // CONFIG_H