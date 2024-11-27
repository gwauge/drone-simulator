#pragma once

#define __USE_XOPEN
#define _GNU_SOURCE

#include <unistd.h> // For usleep
#include <stdlib.h> // For exit
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>
// #include <pthread.h>
#include <semaphore.h>

#define NUM_COMPONENTS 4
#define NUM_OBSTACLES 20
#define NUM_TARGETS 10

#define DELAY 50000 // Microseconds delay for refresh
#define PIPE_NAME_SIZE 25

#define DRONE_SYMBOL "+"
#define OBSTACLE_SYMBOL "*"

#define DRONE_PAIR 1
#define OBSTACLE_PAIR 2
#define TARGET_PAIR 3

#define COMMAND_FORCE 10.0   // Force magnitude for key press
#define MASS 1.0             // Drone mass in kg
#define DAMPING 0.5          // Damping coefficient
#define TIME_STEP 0.1        // Time step for simulation (100 ms)
#define REPULSION_RADIUS 5.0 // Max distance for obstacle repulsion
#define ETA 10.0             // Strength of the repulsive force

typedef struct
{
    int x, y;
} Object;

typedef struct
{
    int n, e, s, w, reset;
} Input;

typedef struct
{
    int lifetime;
    int x, y;
} Obstacle;

typedef struct
{
    int number;
    int x, y;
} Target;

typedef struct
{
    float x, y;
    float vx, vy;
} Drone;

typedef struct
{
    Input input;
    Drone drone;
    Obstacle obstacles[NUM_OBSTACLES];
    Target targets[NUM_TARGETS];
} WorldState;

#define LOGFILE "watchdog.log"

extern sem_t *log_mutex;
void init_mutex();
void cleanup_mutex();
void write_log(const char *message);
void get_current_time(char *buffer, size_t size);
void signal_handler();
void register_signal_handler();
void handle_select_error(int result);
