#!/bin/bash
# Build script for the project.

# check if src folder exists to ensure correct working directory
if [ ! -d "src" ]; then
    echo "Error: src folder not found. Run build.sh from project root."
    exit 1
fi

rm -rf generated
mkdir generated

# generate fastdds types
fastddsgen -flat-output-dir -d generated interfaces/*.idl

# if build folder exists, delete it
if [ -d "build" ]; then
    rm -rf build
fi

# create build folder
mkdir build

cd build

export LD_LIBRARY_PATH=/usr/local/lib/

# build project
cmake ..
make -j8
