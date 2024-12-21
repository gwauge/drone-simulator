#pragma once

#include <ncurses.h>
#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <sys/select.h>

#include "utils.hpp"
#include "pipes.hpp"
#include "drone.hpp"
#include "obstacles.hpp"
#include "targets.hpp"

void init_colors();
void init_ncruses();
void draw_drone(WINDOW *win, Drone *drone);
void draw_obstacles(WINDOW *win, Obstacle *obstacles);
void draw_targets(WINDOW *win, Target *targets);
void display(WINDOW *win, WorldState *world_state);
void reset_input(Input *input);
void send_map_size(Process *process, int LINES, int COLS);
double compute_score(
    double time_elapsed,
    int targets_reached,
    int obstacles_encountered,
    double distance_traveled);
void rotate_fds(int *fds, int count);
void blackboard(
    Process *watchdog_process,
    Process *drone_process,
    Process *obstacle_process,
    Process *targets_process,
    Process *keyboard_process);
