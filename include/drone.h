#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/select.h>

#include "utils.h"
#include "pipes.h"
#include "obstacles.h"

// struct Drone;

Drone make_drone(float x, float y, float vx, float vy);
void update_drone_position(Drone *drone, float fx, float fy);
float calculate_repulsive_force(Obstacle *obstacles, Drone *drone);
void drone_component(int read_fd, int write_fd);
