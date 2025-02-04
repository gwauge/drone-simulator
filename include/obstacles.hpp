#pragma once

#include <ctime>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "utils.hpp"
#include "pipes.hpp"
#include "obstacles_dds.hpp"

// #define OBSTACLE_MAX_LIFETIME 50
// #define OBSTACLE_START_COUNT 5
#define OBSTACLE_UNSET -1
// #define OBSTACLE_SPAWN_CHANCE 5 // 1 in P chance of spawning a new obstacle per time step if there are free slots

// struct Obstacle;

Obstacle make_obstacle(int lifetime, int x, int y);
void addObstacle(
    int COLS,
    int LINES,
    std::vector<long> &obstacles_x,
    std::vector<long> &obstacles_y,
    std::vector<long> &obstacles_lifetime);
void obstacles_component(int read_fd, int write_fd);
