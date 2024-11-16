#include <constants.h>

typedef struct
{
    int x, y;
} Object;

typedef struct
{
    float x, y;   // Position
    float vx, vy; // Velocity
} Drone;

typedef struct
{
    int n, e, s, w, reset;
} Input;

typedef struct
{
    char process_name[50];
    char read_pipe_name[PIPE_NAME_SIZE];
    char write_pipe_name[PIPE_NAME_SIZE];
    int read_fd;
    int write_fd;
} ProcessInfo;
