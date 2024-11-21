cmake_minimum_required(VERSION 3.10)

# Project name
project(drone-simulator)

# Set the C standard
set(CMAKE_C_STANDARD 99)

# Include directories
include_directories(include)

# Source files
set(SRCS
    src/main.c
    src/drone.c
    src/watchdog.c
    src/obstacles.c
    src/blackboard.c
    src/test.c
    src/pipes.c
    src/utils.c
)

# Add executable
add_executable(dronesim ${SRCS})

# Link libraries
target_link_libraries(dronesim ncurses m)

# Add compiler flags
target_compile_options(dronesim PRIVATE -Wall -Wextra)

# Add compiler flags
target_compile_options(dronesim PRIVATE -Wall -Wextra)

# Set build types
set(CMAKE_C_FLAGS_DEBUG "-g")

# Clean files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "watchdog.log")
