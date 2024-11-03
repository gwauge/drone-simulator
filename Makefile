# Variables
SRC_DIR = src
BUILD_DIR = build
TARGET_BLACKBOARD = $(BUILD_DIR)/blackboard
TARGET_DRONE = $(BUILD_DIR)/drone
SRC_BLACKBOARD = $(SRC_DIR)/blackboard.c
SRC_DRONE = $(SRC_DIR)/drone.c

# Compiler
CC = gcc
CFLAGS = -Wall -g -lncurses -lm -I$(SRC_DIR)

# Rules
all: $(TARGET_BLACKBOARD) $(TARGET_DRONE)

$(TARGET_BLACKBOARD): $(SRC_BLACKBOARD)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

$(TARGET_DRONE): $(SRC_DRONE)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET_BLACKBOARD)
	./$(TARGET_BLACKBOARD)

.PHONY: all clean run
