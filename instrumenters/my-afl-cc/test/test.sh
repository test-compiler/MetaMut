#!/bin/bash

CC=$(pwd)/../output/afl-gcc-fast
CXX=$(pwd)/../output/afl-g++-fast
make clean
../scripts/server.py --bbmap-dir bbmap_dir make CC=$CC CXX=$CXX
../scripts/showmap.py --bbmap-dir bbmap_dir -- make run
make clean
rm -rf bbmap_dir
