#include <stdio.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <types.h>
#include <constants.h>
#include <lib.c>

void update_drone_position(Drone *drone, float fx, float fy)
{
    // Update velocities based on force and damping
    drone->vx += (fx - DAMPING * drone->vx) / MASS * TIME_STEP;
    drone->vy += (fy - DAMPING * drone->vy) / MASS * TIME_STEP;

    // Update positions based on velocity
    drone->x += drone->vx * TIME_STEP;
    drone->y += drone->vy * TIME_STEP;
}

float calculate_repulsive_force(Object *obstacle, Drone *drone)
{
    // Calculate distance to obstacle
    float dx = drone->x - obstacle->x;
    float dy = drone->y - obstacle->y;
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
    int COLS, LINES;
    // Get terminal size
    if (read(STDIN_FILENO, &COLS, sizeof(int)) <= 0)
        exit(1);
    if (read(STDIN_FILENO, &LINES, sizeof(int)) <= 0)
        exit(1);

    write(STDOUT_FILENO, "ACK", 3 + 1);

    Drone drone = {0.0, 0.0, 0.0, 0.0};      // Initialize drone at origin with zero velocity
    Input input = {0, 0, 0, 0};              // Initialize input commands
    Object obstacle = {COLS / 3, LINES / 3}; // Place obstacle at a fixed point

    while (1)
    {
        // Read control forces from the pipe (e.g., from blackboard server)
        if (read(STDIN_FILENO, &input, sizeof(Input)) <= 0)
            break;

        // convert input into control forces
        float force_x = 0.0, force_y = 0.0;
        if (input.n)
            force_y -= COMMAND_FORCE;
        if (input.s)
            force_y += COMMAND_FORCE;
        if (input.e)
            force_x += COMMAND_FORCE;
        if (input.w)
            force_x -= COMMAND_FORCE;
        if (input.reset)
        {
            drone.vx = 0;
            drone.vy = 0;
        }

        // Calculate repulsive force from obstacle
        float repulsion_force = calculate_repulsive_force(&obstacle, &drone);

        // Decompose repulsive force into x and y components
        float dx = drone.x - obstacle.x;
        float dy = drone.y - obstacle.y;
        float distance = sqrt(dx * dx + dy * dy);
        if (distance > 0)
        {
            force_x += repulsion_force * (dx / distance);
            force_y += repulsion_force * (dy / distance);
        }

        // Update drone position based on control forces
        update_drone_position(&drone, force_x, force_y);

        // Write updated position and velocity back to the blackboard server
        write(STDOUT_FILENO, &drone, sizeof(Drone));

        // reset force
        force_x = 0.0;
        force_y = 0.0;

        // Simulate time step
        usleep(TIME_STEP * 500000); // Convert TIME_STEP to microseconds
    }

    return 0;
}
