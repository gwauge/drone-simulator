#pragma once

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

#include "utils.h"
#include "pipes.h"

// #define TARGET_START_COUNT 5
#define TARGET_UNSET -1
// #define TARGET_SPAWN_CHANCE 10 // 1 in P chance of spawning a new obstacle per time step if there are free slots

// struct Target;

Target make_target(int number, int x, int y);
void addTarget(int COLS, int LINES, Target *targets, int *free_slots, int *number);
void targets_component(int read_fd, int write_fd);
