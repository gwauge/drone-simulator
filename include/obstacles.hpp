#pragma once

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.hpp"
#include "pipes.hpp"

// #define OBSTACLE_MAX_LIFETIME 50
// #define OBSTACLE_START_COUNT 5
#define OBSTACLE_UNSET -1
// #define OBSTACLE_SPAWN_CHANCE 5 // 1 in P chance of spawning a new obstacle per time step if there are free slots

// struct Obstacle;

Obstacle make_obstacle(int lifetime, int x, int y);
void addObstacle(int COLS, int LINES, Obstacle *obstacles, int *free_slots);
void obstacles_component(int read_fd, int write_fd);
