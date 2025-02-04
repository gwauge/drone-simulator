#include "utils.hpp"

#define LOGFILE "watchdog.log"
#define DT 3 // Inactivity threshold in seconds

sem_t *log_mutex = NULL;

Params global_params; // Define the global variable

int parse_parameters()
{
    FILE *file;
    char jsonBuffer[MAX_FILE_SIZE];

    file = fopen("../appsettings.json", "r");

    if (file == NULL)
    {
        perror("Error opening the file");
        return EXIT_FAILURE; // 1
    }

    int len = fread(jsonBuffer, 1, sizeof(jsonBuffer), file);
    fclose(file);

    cJSON *json = cJSON_Parse(jsonBuffer); // parse the text to json object

    if (json == NULL)
    {
        perror("Error parsing the file");
        return EXIT_FAILURE;
    }

    global_params.debug = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(json, "DEBUG"));
    global_params.mode = cJSON_GetObjectItemCaseSensitive(json, "MODE")->valueint;
    global_params.num_obstacles = cJSON_GetObjectItemCaseSensitive(json, "NUM_OBSTACLES")->valueint;
    if (global_params.num_obstacles > OBSTACLES_MAX_NUMBER)
    {
        printf("Number of obstacles exceeds the maximum limit\n");
        return EXIT_FAILURE;
    }
    global_params.obstacle_max_lifetime = cJSON_GetObjectItemCaseSensitive(json, "OBSTACLE_MAX_LIFETIME")->valueint;
    global_params.obstacle_spawn_chance = cJSON_GetObjectItemCaseSensitive(json, "OBSTACLE_SPAWN_CHANCE")->valueint;
    global_params.obstacle_start_count = cJSON_GetObjectItemCaseSensitive(json, "OBSTACLE_START_COUNT")->valueint;

    global_params.num_targets = cJSON_GetObjectItemCaseSensitive(json, "NUM_TARGETS")->valueint;
    if (global_params.num_targets > TARGETS_MAX_NUMBER)
    {
        printf("Number of targets exceeds the maximum limit\n");
        return EXIT_FAILURE;
    }
    global_params.target_spawn_chance = cJSON_GetObjectItemCaseSensitive(json, "TARGET_SPAWN_CHANCE")->valueint;
    global_params.target_start_count = cJSON_GetObjectItemCaseSensitive(json, "TARGET_START_COUNT")->valueint;

    global_params.delay = cJSON_GetObjectItemCaseSensitive(json, "DELAY")->valueint;
    global_params.drone_symbol = cJSON_GetObjectItemCaseSensitive(json, "DRONE_SYMBOL")->valuestring[0];
    global_params.obstacle_symbol = cJSON_GetObjectItemCaseSensitive(json, "OBSTACLE_SYMBOL")->valuestring[0];
    global_params.command_force = cJSON_GetObjectItemCaseSensitive(json, "COMMAND_FORCE")->valuedouble;
    global_params.mass = cJSON_GetObjectItemCaseSensitive(json, "MASS")->valuedouble;
    global_params.damping = cJSON_GetObjectItemCaseSensitive(json, "DAMPING")->valuedouble;
    global_params.time_step = cJSON_GetObjectItemCaseSensitive(json, "TIME_STEP")->valuedouble;
    global_params.repulsion_radius = cJSON_GetObjectItemCaseSensitive(json, "REPULSION_RADIUS")->valuedouble;
    global_params.eta = cJSON_GetObjectItemCaseSensitive(json, "ETA")->valuedouble;

    if (global_params.debug)
        LOG(&global_params);

    cJSON_Delete(json); // clean

    return EXIT_SUCCESS; // 0
}

void init_mutex()
{
    log_mutex = sem_open("/log_mutex", O_CREAT, 0644, 1);
    if (log_mutex == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    printf("Mutex initialized\n");
}

void cleanup_mutex()
{
    sem_close(log_mutex);
    sem_unlink("/log_mutex");
    printf("Mutex closed\n");
}

void write_log(const char *message)
{
    sem_wait(log_mutex); // Acquire the lock

    FILE *log = fopen(LOGFILE, "a");
    if (log == NULL)
    {
        perror("fopen");
        sem_post(log_mutex); // Release the lock
        exit(EXIT_FAILURE);
    }
    fprintf(log, "%s\n", message);
    fclose(log);

    sem_post(log_mutex); // Release the lock
}

void get_current_time(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void signal_handler(int sig)
{
    char buffer[64];
    get_current_time(buffer, sizeof(buffer));
    char log_message[128];
    snprintf(log_message, sizeof(log_message), "PID: %d, Time: %s", getpid(), buffer);
    write_log(log_message);
}

void register_signal_handler()
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGUSR2); // Block SIGUSR2 signals
    sa.sa_flags = 0;
    sa.sa_handler = signal_handler;

    if (sigaction(SIGUSR1, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void handle_select_error(int result)
{
    if (result == -1)
    {
        perror("select");
        exit(EXIT_FAILURE);
    }
}
