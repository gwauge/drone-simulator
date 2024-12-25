#pragma once

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/signal.h>

typedef struct
{
    int read_fd;  // File descriptor for reading
    int write_fd; // File descriptor for writing
} PipeEnds;

typedef struct
{
    pid_t pid;
    char name[256];
    PipeEnds parent_to_child;
    PipeEnds child_to_parent;

    void (*run)(int read_fd, int write_fd); // function to execute
} Process;

void set_nonblocking(int fd);

// Create a pair of unnamed pipes (one for each direction of communication)
void create_pipe(PipeEnds *parent_to_child, PipeEnds *child_to_parent);

// Fork a process and return the child's PID
pid_t fork_process(
    // PipeEnds *parent_to_child,
    // PipeEnds *child_to_parent
);

// Close unused pipe ends in the parent or child process
void close_unused_pipe_ends(PipeEnds *parent_to_child, PipeEnds *child_to_parent, int is_child);

// Handle pipe errors
void handle_pipe_error(int result, const char *message);
void handle_pipe_read_error(size_t result);
void handle_pipe_write_error(size_t result);

// Write a message to the pipe
void write_to_pipe(int write_fd, const char *message);

// Read a message from the pipe
void read_from_pipe(int read_fd, char *buffer, size_t buffer_size);

Process create_process(const char *name, void (*run)(int read_fd, int write_fd));
void fork_process_and_run(Process *process);
