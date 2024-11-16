#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>

#include "utils.h"

typedef struct
{
    float x, y;   // Position
    float vx, vy; // Velocity
} Drone;

void update_drone_position(Drone *drone, float fx, float fy);
float calculate_repulsive_force(Object *obstacle, Drone *drone);
void drone_component(int read_fd, int write_fd);
