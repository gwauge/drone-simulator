#pragma once

#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <sys/select.h>

#include "utils.h"
#include "pipes.h"
#include "drone.h"

void init_colors();
void init_ncruses();
void draw_drone(Drone *drone);
void draw_obstacles(Object *obstacle);
void display(Drone *drone, Object *obstacle);
void reset_input(Input *input);
void blackboard(
    Process *drone_process,
    Process *watchdog_process);
