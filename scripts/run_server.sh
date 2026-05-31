#!/usr/bin/env sh
set -eu

if [ -x ./build/lightkv_server ]; then
    ./build/lightkv_server
elif [ -x ./build/Debug/lightkv_server ]; then
    ./build/Debug/lightkv_server
else
    echo "lightkv_server not found. Run scripts/build.sh first." >&2
    exit 1
fi

