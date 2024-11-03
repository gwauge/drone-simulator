#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#define MASS 1.0             // Drone mass in kg
#define DAMPING 1.0          // Damping coefficient
#define TIME_STEP 0.1        // Time step for simulation (100 ms)
#define REPULSION_RADIUS 5.0 // Max distance for obstacle repulsion
#define ETA 10.0             // Strength of the repulsive force

typedef struct
{
    float x, y;   // Position
    float vx, vy; // Velocity
} Drone;

void update_drone_position(Drone *drone, float fx, float fy)
{
    // Update velocities based on force and damping
    drone->vx += (fx - DAMPING * drone->vx) / MASS * TIME_STEP;
    drone->vy += (fy - DAMPING * drone->vy) / MASS * TIME_STEP;

    // Update positions based on velocity
    drone->x += drone->vx * TIME_STEP;
    drone->y += drone->vy * TIME_STEP;
}

float calculate_repulsive_force(float obstacle_x, float obstacle_y, float drone_x, float drone_y)
{
    // Calculate distance to obstacle
    float dx = drone_x - obstacle_x;
    float dy = drone_y - obstacle_y;
    float distance = sqrt(dx * dx + dy * dy);

    // Apply repulsion only within a certain radius
    if (distance < REPULSION_RADIUS && distance > 0)
    {
        float force_magnitude = ETA * (1.0 / distance - 1.0 / REPULSION_RADIUS) / (distance * distance);
        return force_magnitude;
    }
    return 0.0;
}

int main()
{
    Drone drone = {0.0, 0.0, 0.0, 0.0}; // Initialize drone at origin with zero velocity

    float force_x = 0.0, force_y = 0.0;

    // Simulate pressing the "up" key (apply upward force)
    float command_force = 1.0; // Force magnitude for key press
    force_y += command_force;

    // Obstacle at (3, 3) coordinates
    float obstacle_x = 3.0;
    float obstacle_y = 3.0;

    for (int t = 0; t < 50; t++)
    { // Run simulation for 100 steps
        // Calculate repulsive force from obstacle
        float repulsion_force = calculate_repulsive_force(obstacle_x, obstacle_y, drone.x, drone.y);

        // Decompose repulsive force into x and y components
        float dx = drone.x - obstacle_x;
        float dy = drone.y - obstacle_y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0)
        {
            force_x += repulsion_force * (dx / distance);
            force_y += repulsion_force * (dy / distance);
        }

        // Update drone position based on total forces
        update_drone_position(&drone, force_x, force_y);

        // Print the position and velocity of the drone
        printf("Time: %.1f s, Position: (%.2f, %.2f), Velocity: (%.2f, %.2f)\n",
               t * TIME_STEP, drone.x, drone.y, drone.vx, drone.vy);

        // Reset command force for next iteration (only apply once)
        force_x = 0.0;
        force_y = 0.0;

        // Wait for the next time step
        usleep(TIME_STEP * 1000000); // Convert TIME_STEP to microseconds
    }

    return 0;
}
