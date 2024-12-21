#include "watchdog.hpp"

int check_logfile(pid_t *pids)
{
    sem_wait(log_mutex); // Acquire the lock

    FILE *log = fopen(LOGFILE, "r");
    if (log == NULL)
    {
        perror("fopen");
        sem_post(log_mutex); // Release the lock
        exit(EXIT_FAILURE);
    }

    char line[128];
    time_t now = time(NULL);
    int active[NUM_COMPONENTS];
    memset(active, 0, sizeof(active));
    // active[0] = 1; // Watchdog is always active

    while (fgets(line, sizeof(line), log))
    {
        int pid;
        char timestamp[64];
        sscanf(line, "PID: %d, Time: %63[^\n]", &pid, timestamp);

        struct tm t;
        strptime(timestamp, "%Y-%m-%d %H:%M:%S", &t);
        time_t log_time = mktime(&t);

        if (difftime(now, log_time) <= DT)
        {
            for (int i = 0; i < NUM_COMPONENTS; i++)
            {
                if (pids[i] == pid)
                {
                    active[i] = 1;
                    break;
                }
            }
        }
    }

    fclose(log);
    sem_post(log_mutex); // Release the lock

    for (int i = 1; i < NUM_COMPONENTS; i++)
    {
        if (!active[i])
        {
            char buffer[64];
            get_current_time(buffer, sizeof(buffer));
            char log_message[128];
            snprintf(log_message, sizeof(log_message), "Process %d inactive. Terminating all processes at %s", pids[i], buffer);
            write_log(log_message);

            for (int j = 1; j < NUM_COMPONENTS; j++)
            {
                kill(pids[j], SIGKILL);
            }

            return 0;
        }
    }

    return 1;
}

void watchdog_component(int read_fd, int write_fd)
{
    // register_signal_handler();

    size_t bytes_size;

    // Get PIDs of all processes from parent
    pid_t pids[NUM_COMPONENTS];
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        bytes_size = read(read_fd, &pids[i], sizeof(pid_t));
        handle_pipe_read_error(bytes_size);
    }

    int active;

    sleep(2); // give time for other processes to start

    while (1)
    {
        signal_handler(SIGUSR1);
        for (int i = 1; i < NUM_COMPONENTS; i++)
        {
            kill(pids[i], SIGUSR1);
        }
        sleep(DT);

        active = check_logfile(pids);

        printf("[watchdog internal] active: %d\n", active);

        // send activity status to parent
        bytes_size = write(write_fd, &active, sizeof(int));
        handle_pipe_write_error(bytes_size);
    }
}
