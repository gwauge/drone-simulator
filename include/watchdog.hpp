#pragma once

#include "utils.hpp"
#include "pipes.hpp"

#define DT 3 // Inactivity threshold in seconds

int check_logfile();
void watchdog_component(int read_fd, int write_fd);
