#pragma once

#include "utils.h"
#include "pipes.h"

#define DT 3 // Inactivity threshold in seconds

int check_logfile();
void watchdog_component(int read_fd, int write_fd);
