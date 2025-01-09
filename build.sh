#!/bin/bash
# This script builds the project using cmake and make.

# Check if build.sh is being run from the correct directory
if [ ! -f "build.sh" ]; then
    echo "Please run build.sh from the root directory of the project."
    exit 1
fi

# Function to display help message
show_help() {
    echo "Usage: build.sh [-j <threads>] [-r]"
    echo ""
    echo "Options:"
    echo "  -j <threads>  Number of threads for parallel building. Directly passed to cmake/make. Defaults to 8."
    echo "  -r            Run the project after build step. Defaults to false."
    echo "  -h            Show this help message."
}

# Default values
threads=8
run_after_build=false

# Parse command line arguments
while getopts "j:rh" opt; do
    case ${opt} in
        j )
            threads=$OPTARG
            ;;
        r )
            run_after_build=true
            ;;
        h )
            show_help
            exit 0
            ;;
        \? )
            show_help
            exit 1
            ;;
    esac
done

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Run cmake and make with the specified number of threads
cmake ..
make -j${threads}

# Run the project if -r option was set
if [ "$run_after_build" = true ]; then
    ./dronesim
fi
