#!/usr/bin/env bash
set -euo pipefail

BENCH="./build/lightkv_bench"

if [ ! -x "$BENCH" ]; then
    echo "lightkv_bench not found. Run 'make' first."
    exit 1
fi

"$BENCH" --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8 "$@"
