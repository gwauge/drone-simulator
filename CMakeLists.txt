cmake_minimum_required(VERSION 3.10)

# Project name
project(drone-simulator LANGUAGES CXX)
# Set the C standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    src/drone.cpp
    src/watchdog.cpp
    src/obstacles.cpp
    src/targets.cpp
    src/blackboard.cpp
    src/pipes.cpp
    src/utils.cpp
    src/keyboard.cpp
)

# Create a library target for the source files
add_library(dronesim_lib ${SRCS})

# FetchContent module
include(FetchContent)

# Fetch cJSON
FetchContent_Declare(
  cjson
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG v1.7.18
)

FetchContent_MakeAvailable(cjson)

# Include cJSON directory
include_directories(${cjson_SOURCE_DIR})

# Link cJSON to the library
target_link_libraries(dronesim_lib cjson)

# Add executable & link libraries
add_executable(dronesim src/main.cpp)
target_link_libraries(dronesim dronesim_lib ncurses m cjson)

# Add compiler flags
target_compile_options(dronesim PRIVATE -Wall -Wextra)


# === MISC ===
# Clean files
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "watchdog.log")
