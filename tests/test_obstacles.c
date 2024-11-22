#include "obstacles.h"

int main()
{
    Obstacle obstacles_component[NUM_OBSTACLES];
    int free_slots = NUM_OBSTACLES;

    addObstacle(10, 10, obstacles_component, &free_slots);

    if (free_slots == NUM_OBSTACLES - 1 && obstacles_component[0].lifetime > 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
