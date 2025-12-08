#!/bin/bash

# Compile C++ core (optional, for standalone testing)
# g++ -O3 -std=c++17 cpp/timetable_solver.cpp -o cpp/timetable_solver

# Build Cython extension
python3 cython/setup.py build_ext --build-lib backend

echo "Compilation complete. Extension placed in backend/"
