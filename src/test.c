#include "test.h"

int test_component()
{
    printf("Test component - PID: %d\n", getpid());
    return 0;
}

// Component1 logic
void run_component1(int read_fd, int write_fd)
{
    pid_t pid = getpid();
    char buffer[256];
    read_from_pipe(read_fd, buffer, sizeof(buffer));
    printf("Component %d received: %s\n", pid, buffer);

    char ack_message[256];
    snprintf(ack_message, sizeof(ack_message), "Acknowledged by Component %d", pid);
    write_to_pipe(write_fd, ack_message);
}
