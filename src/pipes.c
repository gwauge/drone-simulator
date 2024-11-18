#include "pipes.h"

void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL");
    }
}

void create_pipe(PipeEnds *parent_to_child, PipeEnds *child_to_parent)
{
    int pipe1[2]; // Parent -> Child
    int pipe2[2]; // Child -> Parent

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        perror("Failed to create pipes");
        exit(EXIT_FAILURE);
    }

    parent_to_child->read_fd = pipe1[0];
    parent_to_child->write_fd = pipe1[1];

    child_to_parent->read_fd = pipe2[0];
    child_to_parent->write_fd = pipe2[1];
}

pid_t fork_process(
    // PipeEnds *parent_to_child,
    // PipeEnds *child_to_parent
)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("Failed to fork process");
        exit(EXIT_FAILURE);
    }
    return pid;
}

void close_unused_pipe_ends(PipeEnds *parent_to_child, PipeEnds *child_to_parent, int is_child)
{
    if (is_child)
    {
        close(parent_to_child->write_fd); // Close write end in child
        close(child_to_parent->read_fd);  // Close read end in child
    }
    else
    {
        close(parent_to_child->read_fd);  // Close read end in parent
        close(child_to_parent->write_fd); // Close write end in parent
    }
}

void handle_pipe_error(int result, const char *message)
{
    if (result == -1)
    {
        perror(message);
        exit(EXIT_FAILURE);
    }
}

void handle_pipe_read_error(size_t result)
{
    handle_pipe_error(result, "Failed to read from pipe");
}

void handle_pipe_write_error(size_t result)
{
    handle_pipe_error(result, "Failed to write to pipe");
}

void write_to_pipe(int write_fd, const char *message)
{
    ssize_t bytes_read = write(write_fd, message, strlen(message) + 1);
    handle_pipe_write_error(bytes_read);
}

void read_from_pipe(int read_fd, char *buffer, size_t buffer_size)
{
    ssize_t bytes_read = read(read_fd, buffer, buffer_size - 1);
    handle_pipe_read_error(bytes_read);

    buffer[bytes_read] = '\0'; // Null-terminate the string
}

Process create_process(char *name, void (*run)(int read_fd, int write_fd))
{
    Process process;
    strcpy(process.name, name);
    process.run = run;

    // Create pipes
    create_pipe(&process.parent_to_child, &process.child_to_parent);

    return process;
}

void fork_process_and_run(Process *process)
{
    pid_t pid = fork_process(&process->parent_to_child, &process->child_to_parent);
    if (pid == 0)
    {
        // In child process
        close_unused_pipe_ends(&process->parent_to_child, &process->child_to_parent, 1);
        process->run(process->parent_to_child.read_fd, process->child_to_parent.write_fd);
        exit(EXIT_SUCCESS);
    }
    else
    {
        // In parent process
        process->pid = pid;
        close_unused_pipe_ends(&process->parent_to_child, &process->child_to_parent, 0);
    }
}
