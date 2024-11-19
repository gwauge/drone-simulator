#include "obstacles.h"

// Add an obstacle at a random position with a random lifetime
void addObstacle(int COLS, int LINES, Obstacle *obstacles, int *free_slots)
{
    for (int i = 0; i < NUM_OBSTACLES; i++)
    {
        if (obstacles[i].lifetime == OBSTACLE_UNSET)
        {
            obstacles[i] = (Obstacle){
                rand() % OBSTACLE_MAX_LIFETIME + 1, // lifetime
                rand() % COLS,                      // x position
                rand() % LINES,                     // y position
            };

            *free_slots--;
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
    Obstacle obstacles[NUM_OBSTACLES];
    int free_slots = NUM_OBSTACLES;
    for (int i = 0; i < NUM_OBSTACLES; ++i)
    {
        obstacles[i].lifetime = OBSTACLE_UNSET;
    }

    // Add initial obstacles
    for (int i = 0; i < OBSTACLE_START_COUNT; ++i)
    {
        addObstacle(COLS, LINES, obstacles, &free_slots);
    }

    while (1)
    {
        // Check for expired obstacles
        for (int i = 0; i < NUM_OBSTACLES; ++i)
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
            if (rand() % OBSTACLE_SPAWN_CHANCE == 0)
                addObstacle(COLS, LINES, obstacles, &free_slots);
        }

        // Send obstacles to blackboard
        bytes_size = write(write_fd, obstacles, sizeof(Obstacle) * NUM_OBSTACLES);
        handle_pipe_write_error(bytes_size);

        sleep(1); // Sleep for 1 second
    }
}
