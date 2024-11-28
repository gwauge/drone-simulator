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

void send_targets(int write_fd, Target *targets)
{
    ssize_t bytes_size = write(write_fd, targets, sizeof(Target) * NUM_TARGETS);
    handle_pipe_write_error(bytes_size);
}

void targets_component(int read_fd, int write_fd)
{
    register_signal_handler();

    srand(time(NULL));

    fd_set readfds;
    struct timeval timeout;

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

    if (DEBUG)
    {
        printf("[targets] COLS: %d, LINES: %d\n", COLS, LINES);
        targets[0] = make_target(0, 10, 10);
    }

    // Send initial targets to blackboard
    send_targets(write_fd, targets);

    Drone drone;

    while (1)
    {
        FD_ZERO(&readfds);

        // Add the FDs to the fd_set
        FD_SET(read_fd, &readfds);

        timeout.tv_sec = 0; // Wait for up to 5 seconds
        timeout.tv_usec = 0;

        // check if a collision has occurred
        int result = select(read_fd + 1, &readfds, NULL, NULL, &timeout);
        handle_select_error(result);
        if (result > 0)
        {
            if (FD_ISSET(read_fd, &readfds))
            {
                int collision_idx;
                bytes_size = read(read_fd, &collision_idx, sizeof(int));
                handle_pipe_read_error(bytes_size);

                if (collision_idx >= 0 && collision_idx < NUM_TARGETS && targets[collision_idx].number != TARGET_UNSET)
                    if (DEBUG)
                    {
                        printf("[targets] received detected collision with target %d\n", collision_idx);
                    }
                {
                    // Reset target
                    targets[collision_idx].number = TARGET_UNSET;
                    free_slots += 1;

                    // Send targets to blackboard
                    send_targets(write_fd, targets);
                }
            }
        }

        // Add new targets
        if (free_slots > 0)
        {
            // with a chance of 1 in P add an obstacle
            if (rand() % TARGET_SPAWN_CHANCE == 0)
            {
                addTarget(COLS, LINES, targets, &free_slots, &number);

                // Send targets to blackboard
                send_targets(write_fd, targets);
            }
        }

        sleep(1); // Sleep for 1 second
    }
}
