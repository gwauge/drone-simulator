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
#include <semaphore.h>

#include <cJSON.h>

#define NUM_COMPONENTS 4
#define OBSTACLES_MAX_NUMBER 256
#define TARGETS_MAX_NUMBER 256

#define DELAY 50000 // Microseconds delay for refresh

#define DRONE_PAIR 1
#define OBSTACLE_PAIR 2
#define TARGET_PAIR 3

typedef struct
{
    int debug;

    int num_obstacles;
    char obstacle_symbol;
    int obstacle_max_lifetime;
    int obstacle_spawn_chance;
    int obstacle_start_count;

    int num_targets;
    int target_spawn_chance;
    int target_start_count;

    int delay;
    char drone_symbol;
    float command_force;
    float mass;
    float damping;
    float time_step;
    float repulsion_radius;
    float eta;
} Parameters;

extern Parameters global_params;

#define MAX_LINE_LENGTH 100
#define MAX_FILE_SIZE 1024
#define USE_MACRO_LOG 0

static inline __attribute__((always_inline)) void logConfig(const Parameters *config)
{
    printf("Parameters:\n");
    printf("\tDebug mode: %d\n", config->debug);

    printf("\tNum obsbtacles: %d\n", config->num_obstacles);
    printf("\tObstacle max lifetime: %d\n", config->obstacle_max_lifetime);
    printf("\tObstacle spawn chance: %d\n", config->obstacle_spawn_chance);
    printf("\tObstacle start count: %d\n", config->obstacle_start_count);

    printf("\tNum targets: %d\n", config->num_targets);
    printf("\tTarget spawn chance: %d\n", config->target_spawn_chance);
    printf("\tTarget start count: %d\n", config->target_start_count);

    printf("\tDelay: %d\n", config->delay);
    printf("\tDrone symbol: %c\n", config->drone_symbol);
    printf("\tObstacle symbol: %c\n", config->obstacle_symbol);
    printf("\tCommand force: %.1f\n", config->command_force);
    printf("\tMass: %.1f\n", config->mass);
    printf("\tDamping: %.1f\n", config->damping);
    printf("\tTime step: %.1f\n", config->time_step);
    printf("\tRepulsion radius: %.1f\n", config->repulsion_radius);
    printf("\tETA: %.1f\n", config->eta);
}

#if USE_MACRO_LOG
#define LOGCONFIG(config)                                      \
    {                                                          \
        printf("DEBUG: %d\n", config->debug);                  \
        printf("Num obsbtacles: %d\n", config->num_obstacles); \
    }
#endif

#ifdef LOGCONFIG
#define LOG(conf)                                    \
    {                                                \
        printf("[%d] at %s:\n", __LINE__, __FILE__); \
        LOGCONFIG(conf);                             \
    }
#else
#define LOG(conf)                                    \
    {                                                \
        printf("[%d] at %s:\n", __LINE__, __FILE__); \
        logConfig(&global_params);                   \
    }
#endif

int parse_parameters();

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
    Obstacle obstacles[OBSTACLES_MAX_NUMBER];
    Target targets[TARGETS_MAX_NUMBER];
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
