#include "utils.h"

#define LOGFILE "watchdog.log"
#define DT 3 // Inactivity threshold in seconds

sem_t *log_mutex = NULL;

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

void signal_handler()
{
    char buffer[64];
    get_current_time(buffer, sizeof(buffer));
    char log_message[128];
    snprintf(log_message, sizeof(log_message), "PID: %d, Time: %s", getpid(), buffer);
    write_log(log_message);
}

void register_signal_handler()
{
    signal(SIGUSR1, signal_handler);
}

void handle_select_error(int result)
{
    if (result == -1)
    {
        perror("select");
        exit(EXIT_FAILURE);
    }
}
