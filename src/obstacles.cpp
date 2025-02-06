#include "obstacles.hpp"

Obstacle make_obstacle(int lifetime, int x, int y)
{
    Obstacle obstacle = {lifetime, x, y};
    return obstacle;
}

// Add an obstacle at a random position with a random lifetime
void addObstacle(
    int COLS,
    int LINES,
    std::vector<int32_t> &obstacles_x,
    std::vector<int32_t> &obstacles_y,
    std::vector<int32_t> &obstacles_lifetime)
{
    obstacles_lifetime.push_back(rand() % global_params.obstacle_max_lifetime + 1);
    obstacles_x.push_back(rand() % COLS);
    obstacles_y.push_back(rand() % LINES);
}

std::atomic<bool> running(true);

void my_signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        running = false;
    }
}

void register_my_signal_handler()
{
    struct sigaction sa;
    sa.sa_handler = my_signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
}

void obstacles_component(int read_fd, int write_fd)
{
    register_signal_handler();

    // initialize obstacles
    std::vector<int32_t> obstacles_x = std::vector<int32_t>();
    std::vector<int32_t> obstacles_y = std::vector<int32_t>();
    std::vector<int32_t> obstacles_lifetime = std::vector<int32_t>();

    srand(time(NULL));

    // Initialize FastDDS
    register_my_signal_handler();

    std::string topic_name = "obstacles";
    if (global_params.mode != 1)
    {
        topic_name += "_local";
    }
    DDSPublisher<Obstacles, ObstaclesPubSubType> *mypub = new DDSPublisher<Obstacles, ObstaclesPubSubType>("obstacles");
    mypub->init();

    ssize_t bytes_size;

    // Get terminal sizeÄ
    int COLS, LINES;
    bytes_size = read(read_fd, &COLS, sizeof(int));
    handle_pipe_read_error(bytes_size);

    bytes_size = read(read_fd, &LINES, sizeof(int));
    handle_pipe_read_error(bytes_size);

    // Add initial obstacles
    for (int i = 0; i < global_params.obstacle_start_count; ++i)
    {
        addObstacle(COLS, LINES, obstacles_x, obstacles_y, obstacles_lifetime);
    }

    Obstacles obstacles;
    while (running)
    {
        // Check for expired obstacles and remove them
        for (int i = 0; i < obstacles_lifetime.size();)
        {
            if (obstacles_lifetime[i] <= 0)
            {
                obstacles_lifetime.erase(obstacles_lifetime.begin() + i);
                obstacles_x.erase(obstacles_x.begin() + i);
                obstacles_y.erase(obstacles_y.begin() + i);
            }
            else
            {
                obstacles_lifetime[i]--;
                ++i;
            }
        }

        // with a chance of 1 in P add an obstacle
        if (rand() % global_params.obstacle_spawn_chance == 0)
            addObstacle(COLS, LINES, obstacles_x, obstacles_y, obstacles_lifetime);

        // send using DDS
        obstacles.obstacles_x(obstacles_x);
        obstacles.obstacles_y(obstacles_y);
        obstacles.obstacles_number(obstacles_x.size());
        mypub->publish(&obstacles);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Time step
    }

    // Clean up
    delete mypub;
    exit(EXIT_SUCCESS);
}
