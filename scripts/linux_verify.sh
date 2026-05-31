#!/usr/bin/env sh
set -eu

cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure

if [ -x ./build/lightkv_server ]; then
    ./build/lightkv_server
elif [ -x ./build/Debug/lightkv_server ]; then
    ./build/Debug/lightkv_server
else
    echo "lightkv_server not found after build." >&2
    exit 1
fi

