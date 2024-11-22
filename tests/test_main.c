#include "drone.h"
#include "obstacles.h"

int test_obstacles()
{
    Obstacle obstacles_component[NUM_OBSTACLES];
    int free_slots = NUM_OBSTACLES;

    addObstacle(10, 10, obstacles_component, &free_slots);

    if (free_slots == NUM_OBSTACLES - 1 && obstacles_component[0].lifetime > 0)
        return 0;
    else
        return 1;
}

int test_drone()
{
    Drone drone = make_drone(0, 0, 0, 0);
    Obstacle obstacle = make_obstacle(0, REPULSION_RADIUS + 1, 0);

    float result = calculate_repulsive_force(&obstacle, &drone);

    if (result == 0.0)
        return 0;
    else
        return 1;
}

int main()
{
    if (test_obstacles() == 0 && test_drone() == 0)
        return 0;
    else
        return 1;
}
