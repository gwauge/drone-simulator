#include <sys/wait.h> // for wait

#include "pipes.h"
#include "test.h"
#include "blackboard.h"
#include "drone.h"

#define NUM_COMPONENTS 2

int main()
{
    // size_t bytes_size;

    // Create processes
    Process processes[NUM_COMPONENTS];

    Process p1 = create_process("Component 1", run_component1);
    processes[0] = p1;

    Process drone_process = create_process("drone", drone_component);
    processes[1] = drone_process;

    // Fork processes
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        fork_process_and_run(&processes[i]);
    }

    // === PARENT ===

    // Communicate with components
    test_component(); // parent

    // Communicate with mock component
    write_to_pipe(p1.parent_to_child.write_fd, "Hello Component!");
    char buffer[256];
    read_from_pipe(p1.child_to_parent.read_fd, buffer, sizeof(buffer));
    printf("Parent received from %s: %s\n", p1.name, buffer);

    blackboard(&drone_process);

    // kill all child processes
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        kill(processes[i].pid, SIGINT);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        wait(NULL);
    }

    return 0;
}
