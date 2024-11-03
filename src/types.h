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
