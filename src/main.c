#include <sys/wait.h> // for wait

#include "utils.h"
#include "pipes.h"
#include "blackboard.h"
#include "watchdog.h"
#include "drone.h"
#include "obstacles.h"
#include "targets.h"

Process processes[NUM_COMPONENTS];

void shutdown()
{
    printf("Shutting down all children...\n");
    // kill all child processes
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        printf("Interrupting process %s (%d)\n", processes[i].name, processes[i].pid);
        kill(processes[i].pid, SIGINT);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        wait(NULL);
    }

    // Cleanup mutex
    cleanup_mutex();
}

int main()
{
    printf("Starting main process with PID %d\n", getpid());

    if (parse_parameters() != 0)
    {
        shutdown();
        exit(EXIT_FAILURE);
    }

    size_t bytes_size;

    // Initialize mutex
    init_mutex();

    // === CHILDREN ===
    Process watchdog_process = create_process("watchdog", watchdog_component);
    processes[0] = watchdog_process;

    Process drone_process = create_process("drone", drone_component);
    processes[1] = drone_process;

    Process obstacle_process = create_process("obstacles", obstacles_component);
    processes[2] = obstacle_process;

    Process targets_process = create_process("targets", targets_component);
    processes[3] = targets_process;

    // Fork processes
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        fork_process_and_run(&processes[i]);
    }

    // === PARENT ===
    signal(SIGINT, shutdown);

    // Communicate with watchdog
    pid_t pids[NUM_COMPONENTS];
    // create list of all pids
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        pids[i] = processes[i].pid;
    }
    bytes_size = write(watchdog_process.parent_to_child.write_fd, &pids, sizeof(pids));
    handle_pipe_write_error(bytes_size);

    blackboard(&watchdog_process, &drone_process, &obstacle_process, &targets_process);

    shutdown();

    return 0;
}
