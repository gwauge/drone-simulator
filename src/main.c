#include <sys/wait.h> // for wait

#include "pipes.h"
#include "test.h"
// #include "blackboard.h"
#include "drone.h"

#define NUM_COMPONENTS 2

int main()
{
    size_t bytes_size;

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
    // Close unused pipe ends
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        close_unused_pipe_ends(&processes[i].parent_to_child, &processes[i].child_to_parent, 0);
    }

    // Communicate with components
    test_component(); // parent

    // Communicate with mock component
    write_to_pipe(p1.parent_to_child.write_fd, "Hello Component!");
    char buffer[256];
    read_from_pipe(p1.child_to_parent.read_fd, buffer, sizeof(buffer));
    printf("Parent received from %s: %s\n", p1.name, buffer);

    // send map size (COLS, LINES) once at the beginning
    int COLS = 1920;
    int LINES = 1080;

    bytes_size = write(drone_process.parent_to_child.write_fd, &COLS, sizeof(int));
    handle_pipe_write_error(bytes_size);
    bytes_size = write(drone_process.parent_to_child.write_fd, &LINES, sizeof(int));
    handle_pipe_write_error(bytes_size);

    // read ACK from drone
    char ack_message[256];
    read_from_pipe(drone_process.child_to_parent.read_fd, ack_message, sizeof(ack_message));
    printf("Parent received from drone: %s\n", ack_message);

    Input input = {0, 1, 1, 0, 0};
    Drone drone;
    while (1)
    {
        // write keyboard input to drone
        bytes_size = write(drone_process.parent_to_child.write_fd, &input, sizeof(Input));
        handle_pipe_write_error(bytes_size);

        // read drone position
        bytes_size = read(drone_process.child_to_parent.read_fd, &drone, sizeof(Drone));
        handle_pipe_read_error(bytes_size);

        printf("Drone position: x=%f, y=%f\n", drone.x, drone.y);
        usleep(1200000);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        wait(NULL);
    }

    return 0;
}
