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
    echo "  -g            Skip auto-generation of fastdds types."
    echo "  -c            Clean the build directory."
}

# Default values
threads=8
run_after_build=false
skip_gen=false
clean=false

# Parse command line arguments
while getopts "j:rhgc" opt; do
    case ${opt} in
        j )
            threads=$OPTARG
            ;;
        r )
            run_after_build=true
            ;;
        g )
            skip_gen=true
            ;;
        c )
            clean=true
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

if [ "$skip_gen" = false ]; then
    # Check if fastddsgen is installed
    if ! command -v fastddsgen &> /dev/null; then
        echo "fastddsgen is not installed. Please install it before running this script."
        exit 1
    fi

    rm -rf generated
    mkdir generated

    # generate fastdds types
    fastddsgen -flat-output-dir -d generated interfaces/*.idl
fi

# if build folder exists, delete it
if [ "$clean" = true ] && [ -d "build" ]; then
    rm -rf build
fi

# create build folder
mkdir -p build

cd build

export LD_LIBRARY_PATH=/usr/local/lib/

# build project
cmake ..
make -j${threads}

# Run the project if -r option was set
if [ "$run_after_build" = true ]; then
    ./dronesim
fi
