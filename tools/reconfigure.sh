#!/bin/bash

cd ../build/
rm -rf ./CMakeCache.txt ./CMakeFiles/ ./cmake_install.cmake ./Makefile ./external/ ./bin/ ./lib/ ./tests/ ./src ./samples

./configure.sh
