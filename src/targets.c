#include "targets.h"

Target make_target(int number, int x, int y)
{
    Target t = {number, x, y};
    return t;
}

// Add a target at a random position
void addTarget(int COLS, int LINES, Target *targets, int *free_slots, int *number)
{
    for (int i = 0; i < NUM_OBSTACLES; i++)
    {
        if (targets[i].number == TARGET_UNSET)
        {
            targets[i] = make_target(
                *number,
                rand() % COLS,
                rand() % LINES);

            *free_slots -= 1;

            *number += 1;
            // make sure number is not too large
            if (*number > NUM_TARGETS - 1)
                *number = 0;

            break;
        }
    }
}

void targets_component(int read_fd, int write_fd)
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

    // Initialize targets
    Target targets[NUM_TARGETS];
    int free_slots = NUM_TARGETS;
    int number = 0;
    for (int i = 0; i < NUM_TARGETS; ++i)
    {
        targets[i].number = TARGET_UNSET;
    }

    // Add initial targets
    for (int i = 0; i < TARGET_START_COUNT; ++i)
    {
        addTarget(COLS, LINES, targets, &free_slots, &number);
    }

    Drone drone;

    while (1)
    {
        bytes_size = read(read_fd, &drone, sizeof(Drone));
        handle_pipe_read_error(bytes_size);

        // check for collisions
        for (int i = 0; i < NUM_TARGETS; i++)
        {
            if (targets[i].number != TARGET_UNSET)
            {
                if (targets[i].x == (int)drone.x && targets[i].y == (int)drone.y)
                {
                    // remove target
                    targets[i].number = TARGET_UNSET;
                    free_slots++;
                }
            }
        }

        // Add new targets
        if (free_slots > 0)
        {
            // with a chance of 1 in P add an obstacle
            if (rand() % TARGET_SPAWN_CHANCE == 0)
                addTarget(COLS, LINES, targets, &free_slots, &number);
        }

        // Send targets to blackboard
        bytes_size = write(write_fd, targets, sizeof(Target) * NUM_TARGETS);
        handle_pipe_write_error(bytes_size);
    }
}
