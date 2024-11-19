CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -lncurses -lm
SRCS = src/main.c src/drone.c src/watchdog.c src/obstacles.c src/blackboard.c src/test.c src/pipes.c src/utils.c
OBJS = $(SRCS:src/%.c=build/%.o)
TARGET = build/dronesim

.DEFAULT_GOAL := default

default: clean all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

build/%.o: src/%.c
	@mkdir -p build
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf build
	rm -f watchdog.log

run: clean $(TARGET)
	./$(TARGET)

.PHONY: all clean run default
