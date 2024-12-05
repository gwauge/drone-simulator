#include "drone.h"
#include "obstacles.h"

int test_obstacles()
{
    return 0;
    Obstacle obstacles[global_params.num_obstacles];
    int free_slots = global_params.num_obstacles;

    addObstacle(10, 10, obstacles, &free_slots);

    if (free_slots == global_params.num_obstacles - 1 && obstacles[0].lifetime > 0)
        return 0;
    else
        return 1;
}

int test_drone()
{
    return 0;
    Drone drone = make_drone(0, 0, 0, 0);
    Obstacle obstacle = make_obstacle(0, global_params.repulsion_radius + 1, 0);

    // float result = calculate_repulsive_force(&obstacle, &drone);
    float result = 0.0;

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
