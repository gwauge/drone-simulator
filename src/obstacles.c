#include "obstacles.h"

Obstacle make_obstacle(int lifetime, int x, int y)
{
    Obstacle obstacle = {lifetime, x, y};
    return obstacle;
}

// Add an obstacle at a random position with a random lifetime
void addObstacle(int COLS, int LINES, Obstacle *obstacles, int *free_slots)
{
    for (int i = 0; i < global_params.num_obstacles; i++)
    {
        if (obstacles[i].lifetime == OBSTACLE_UNSET)
        {
            obstacles[i] = make_obstacle(
                rand() % global_params.obstacle_max_lifetime + 1,
                rand() % COLS,
                rand() % LINES);

            *free_slots -= 1;
            break;
        }
    }
}

void obstacles_component(int read_fd, int write_fd)
{
    register_signal_handler();

    srand(time(NULL));

    ssize_t bytes_size;

    // Get terminal size
    int COLS, LINES;
    bytes_size = read(read_fd, &COLS, sizeof(int));
    handle_pipe_read_error(bytes_size);

    bytes_size = read(read_fd, &LINES, sizeof(int));
    handle_pipe_read_error(bytes_size);

    // Initialize obstacles
    Obstacle obstacles[global_params.num_obstacles];
    int free_slots = global_params.num_obstacles;
    for (int i = 0; i < global_params.num_obstacles; ++i)
    {
        obstacles[i].lifetime = OBSTACLE_UNSET;
    }

    // Add initial obstacles
    for (int i = 0; i < global_params.obstacle_start_count; ++i)
    {
        addObstacle(COLS, LINES, obstacles, &free_slots);
    }

    while (1)
    {
        // Check for expired obstacles
        for (int i = 0; i < global_params.num_obstacles; ++i)
        {
            if (obstacles[i].lifetime == 0)
            {
                obstacles[i].lifetime = OBSTACLE_UNSET;
                free_slots++;
            }
            else if (obstacles[i].lifetime > 0)
            {
                obstacles[i].lifetime--;
            }
        }

        // Add new obstacles
        if (free_slots > 0)
        {
            // with a chance of 1 in P add an obstacle
            if (rand() % global_params.obstacle_spawn_chance == 0)
                addObstacle(COLS, LINES, obstacles, &free_slots);
        }

        // Send obstacles to blackboard
        bytes_size = write(write_fd, obstacles, sizeof(Obstacle) * global_params.num_obstacles);
        handle_pipe_write_error(bytes_size);

        sleep(1); // Sleep for 1 second
    }
}
