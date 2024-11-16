#include <constants.h>

void create_read_pipe_name(char *pipe_name, char *process_name)
{
    snprintf(pipe_name, PIPE_NAME_SIZE, "/tmp/pipe_%s_read", process_name);
}

void create_write_pipe_name(char *pipe_name, char *process_name)
{
    snprintf(pipe_name, PIPE_NAME_SIZE, "/tmp/pipe_%s_write", process_name);
}
