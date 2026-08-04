CC = gcc
CFLAGS = -Wall -Wextra -O3 -fopenmp -Iinclude
LDFLAGS = -fopenmp -lm

SRC_DIR = src
BIN_DIR = bin
INC_DIR = include

SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/parser.c $(SRC_DIR)/rule.c $(SRC_DIR)/grid.c $(SRC_DIR)/automata.c $(SRC_DIR)/daily.c
OBJECTS = $(BIN_DIR)/main.o $(BIN_DIR)/parser.o $(BIN_DIR)/rule.o $(BIN_DIR)/grid.o $(BIN_DIR)/automata.o $(BIN_DIR)/daily.o
HEADERS = $(INC_DIR)/config.h $(INC_DIR)/parser.h $(INC_DIR)/rule.h $(INC_DIR)/grid.h $(INC_DIR)/automata.h $(INC_DIR)/daily.h
TARGET = cellular_automata

.PHONY: all clean test test_circular help

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Added HEADERS as dependency to ensure recompilation when headers change
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

test: $(TARGET)
	./$(TARGET) --test

test_circular: $(TARGET)
	./$(TARGET) --test-circular

clean:
	rm -rf $(BIN_DIR) $(TARGET) output output_circular

help:
	@echo "Cellular Automata Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make               - Compile the program"
	@echo "  make clean         - Remove compiled files and output"
	@echo "  make test          - Run test simulation"
	@echo "  make test_circular - Run test simulation with circular neighborhood (NC)"
	@echo "  make help          - Show this help"
