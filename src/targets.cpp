#include "targets.hpp"

Target make_target(int number, int x, int y)
{
    Target t = {number, x, y};
    return t;
}

// Add a target at a random position
void addTarget(int COLS, int LINES, Target *targets, int *free_slots, int *number)
{
    for (int i = 0; i < global_params.num_targets; i++)
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
            if (*number > global_params.num_targets - 1)
                *number = 0;

            break;
        }
    }
}

void send_targets(int write_fd, Target *targets)
{
    ssize_t bytes_size = write(write_fd, targets, sizeof(Target) * global_params.num_targets);
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
    Target targets[global_params.num_targets];
    int free_slots = global_params.num_targets;
    int number = 0;
    for (int i = 0; i < global_params.num_targets; ++i)
    {
        targets[i].number = TARGET_UNSET;
    }

    // Add initial targets
    for (int i = 0; i < global_params.target_start_count; ++i)
    {
        addTarget(COLS, LINES, targets, &free_slots, &number);
    }

    if (global_params.debug)
    {
        printf("[targets] COLS: %d, LINES: %d\n", COLS, LINES);
        targets[0] = make_target(0, 10, 10);
    }

    // Send initial targets to blackboard
    send_targets(write_fd, targets);

    Drone drone;

    int collision_idx;
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
                bytes_size = read(read_fd, &collision_idx, sizeof(int));
                handle_pipe_read_error(bytes_size);

                if (collision_idx >= 0 && collision_idx < global_params.num_targets && targets[collision_idx].number != TARGET_UNSET)
                {
                    if (global_params.debug)
                    {
                        printf("[targets] received detected collision with target %d\n", collision_idx);
                    }

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
            if (rand() % global_params.target_spawn_chance == 0)
            {
                addTarget(COLS, LINES, targets, &free_slots, &number);

                // Send targets to blackboard
                send_targets(write_fd, targets);
            }
        }

        sleep(1); // Sleep for 1 second
    }
}
