#define DELAY 50000 // Microseconds delay for refresh

#define DRONE "+"
#define OBSTACLE "*"

#define DRONE_PAIR 1
#define OBSTACLE_PAIR 2
#define TARGET_PAIR 3

#define COMMAND_FORCE 10.0   // Force magnitude for key press
#define MASS 1.0             // Drone mass in kg
#define DAMPING 0.5          // Damping coefficient
#define TIME_STEP 0.1        // Time step for simulation (100 ms)
#define REPULSION_RADIUS 5.0 // Max distance for obstacle repulsion
#define ETA 10.0             // Strength of the repulsive force
