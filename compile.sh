#!/bin/bash
# Builds the PDFast-API binary. Extra flags can be passed through CXXFLAGS.

set -euo pipefail

start_time=$(date +%s)

g++ main.cpp src/cpp/*.cpp \
    -std=c++20 -O2 -DNDEBUG -Wall -Wextra -Werror -pedantic ${CXXFLAGS:-} \
    -lredis++ -lhiredis -lssl -lcrypto -lpthread \
    -o app

echo "Compilation finished: $(( $(date +%s) - start_time ))s elapsed."
