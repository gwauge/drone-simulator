#pragma once

#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <sys/select.h>

#include "utils.h"
#include "pipes.h"
#include "drone.h"
#include "obstacles.h"
#include "targets.h"

void init_colors();
void init_ncruses();
void draw_drone(Drone *drone);
void draw_obstacles(Obstacle *obstacles);
void draw_targets(Target *targets);
void display(WorldState *world_state);
void reset_input(Input *input);
void blackboard(
    Process *watchdog_process,
    Process *drone_process,
    Process *obstacle_process,
    Process *targets_process);
