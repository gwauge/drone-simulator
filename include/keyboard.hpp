#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>

#include "utils.hpp"
#include "pipes.hpp"

#define POLL_INTERVAL 100000 // 100ms

void handle_sigint(int sig);
void reset_terminal();
void enable_raw_mode();
void disable_raw_mode();

void keyboard_component(int read_fd, int write_fd);
