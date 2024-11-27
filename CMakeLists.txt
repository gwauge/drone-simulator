cmake_minimum_required(VERSION 3.10)

# Project name
project(drone-simulator LANGUAGES C)
# Set the C standard
set(CMAKE_C_STANDARD 11)

option(CI_BUILD "Set to ON for complete build in CI." OFF)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified. Defaulting to Debug.\n\tUse `cmake -DCMAKE_BUILD_TYPE=Release ..` to build an optimized version.")
    add_compile_options(-g)
endif ()

if (CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Building an optimized release version.")
    add_compile_options(-O3)
endif ()

# === MAIN EXECUTABLE ===
# Include directories
include_directories(include)

# Source files
set(SRCS
    src/drone.c
    src/watchdog.c
    src/obstacles.c
    src/targets.c
    src/blackboard.c
    src/pipes.c
    src/utils.c
)

# Create a library target for the source files
add_library(dronesim_lib ${SRCS})

# Add executable & link libraries
add_executable(dronesim src/main.c)
target_link_libraries(dronesim dronesim_lib ncurses m)

# Add compiler flags
target_compile_options(dronesim PRIVATE -Wall -Wextra)

# === TESTING ===
# Add executable & link libraries
add_executable(runTests tests/test_main.c)
target_link_libraries(runTests PRIVATE dronesim_lib ncurses m)

# Enable testing
enable_testing()

# Register the test with CTest
add_test(
    NAME DroneSimTest
    COMMAND $<TARGET_FILE:runTests>
)

# === MISC ===
# Clean files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "watchdog.log")
