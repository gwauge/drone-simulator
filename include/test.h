#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for pid_t

#include "pipes.h"

int test_component();
void run_component1(int read_fd, int write_fd);
