#!/bin/bash

# prepare
mkdir -p output
cd output
cmake ..
make

# usage: build.sh [gcc|clang|clang-proj] source_dir
