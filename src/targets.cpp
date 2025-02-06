#include "targets.hpp"

Target make_target(int number, int x, int y)
{
    Target t = {number, x, y};
    return t;
}

// Add a target at a random position
void addTarget(int COLS, int LINES, std::vector<int32_t> &targets_x, std::vector<int32_t> &targets_y)
{
    int32_t x = rand() % COLS;
    int32_t y = rand() % LINES;

    targets_x.push_back(x);
    targets_y.push_back(y);
}

std::atomic<bool> targets_running(true);

void my_targets_signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        targets_running = false;
    }
}

void targets_component(int read_fd, int write_fd)
{
    register_signal_handler();

    srand(time(NULL));

    struct sigaction sa;
    sa.sa_handler = my_targets_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    DDSPublisher<Targets, TargetsPubSubType> *mypub = new DDSPublisher<Targets, TargetsPubSubType>("targets");
    mypub->init();

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
    Targets my_targets;
    std::vector<int32_t> targets_x;
    std::vector<int32_t> targets_y;

    // Add initial targets
    for (int i = 0; i < global_params.target_start_count; ++i)
    {
        addTarget(COLS, LINES, targets_x, targets_y);
    }
    bool send_update = true; // send once in the beginning

    if (global_params.debug)
    {
        printf("[targets] COLS: %d, LINES: %d\n", COLS, LINES);
        targets_x.push_back(10);
        targets_y.push_back(10);
    }

    Drone drone;

    int collision_idx;
    while (targets_running)
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

                if (collision_idx >= 0 && collision_idx < targets_x.size())
                {
                    if (global_params.debug)
                    {
                        printf("[targets] received detected collision with target %d\n", collision_idx + 1);
                    }

                    // remove target from array
                    targets_x.erase(targets_x.begin() + collision_idx);
                    targets_y.erase(targets_y.begin() + collision_idx);

                    // Send targets to blackboard
                    send_update = true;
                }
            }
        }

        // with a chance of 1 in P add an obstacle
        if (targets_x.size() < global_params.num_targets && rand() % global_params.target_spawn_chance == 0)
        {
            addTarget(COLS, LINES, targets_x, targets_y);

            // Send targets to blackboard
            send_update = true;
        }

        if (send_update)
        {
            my_targets.targets_number(targets_x.size());
            my_targets.targets_x(targets_x);
            my_targets.targets_y(targets_y);

            mypub->publish(&my_targets);
            send_update = false;
        }

        sleep(1); // Sleep for 1 second
    }

    delete mypub;
    exit(EXIT_SUCCESS);
}
