#pragma once

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>

#include "utils.hpp"
#include "pipes.hpp"
#include "dds.hpp"

#define TARGETS_TOPIC_NAME "targets"

Target make_target(int number, int x, int y);
void addTarget(int COLS, int LINES, std::vector<int32_t> &targets_x, std::vector<int32_t> &targets_y);
void targets_component(int read_fd, int write_fd);
