#include <sys/wait.h> // for wait

#include "utils.h"
#include "pipes.h"
#include "test.h"
#include "blackboard.h"
#include "watchdog.h"
#include "drone.h"

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

    size_t bytes_size;

    // Initialize mutex
    init_mutex();

    // === CHILDREN ===
    Process watchdog_process = create_process("watchdog", watchdog_component);
    processes[0] = watchdog_process;

    Process drone_process = create_process("drone", drone_component);
    processes[1] = drone_process;

    // Fork processes
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        fork_process_and_run(&processes[i]);
    }

    // === PARENT ===
    signal(SIGINT, shutdown);

    // Communicate with components
    test_component(); // parent

    // Communicate with watchdog
    pid_t pids[NUM_COMPONENTS];
    // create list of all pids
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        pids[i] = processes[i].pid;
    }
    bytes_size = write(watchdog_process.parent_to_child.write_fd, &pids, sizeof(pids));
    handle_pipe_write_error(bytes_size);

    blackboard(&drone_process, &watchdog_process);

    shutdown();

    return 0;
}
