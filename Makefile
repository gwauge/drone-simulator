# Variables
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/blackboard
SRC = $(SRC_DIR)/blackboard.c

# Compiler
CC = gcc
CFLAGS = -Wall -g -lncurses -lm

# Rules
all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
