# Development Log

## Stage 0

Created the project scaffold, CMake layout, Makefile, source/include/tests/docs directories, and a minimal server entry point.

Validation focus: repository layout and basic build structure.

## Stage 1

Added `Status`, `Entry`, and thread-safe `KVStore`. Implemented set/get/del/exists/size/clear on top of `std::unordered_map`.

Validation focus: assert-based KVStore unit tests.

## Stage 2

Added `Command`, `Parser`, `Response`, and `CommandExecutor`. Added `lightkv_cli` for local command execution.

Validation focus: parser and command executor tests.

## Stage 3

Added `Buffer`, `TcpConnection`, and Linux socket + epoll `TcpServer`.

Validation focus: Buffer tests on Windows and Linux TCP validation on Ubuntu.

## Stage 4

Added TTL support with `EXPIRE`, `TTL`, lazy deletion, and periodic cleanup.

Validation focus: TTL before/after expiration, CommandExecutor TTL behavior, and regression tests for the TTL positive GET bug.

## Stage 5

Added `LRUCache` and integrated max key capacity into `KVStore`. Added eviction counters and `INFO`.

Validation focus: LRU touch/remove/eviction, GET refresh, SET refresh, DEL and expiration cleanup.

## Stage 6

Added WAL persistence and restart recovery. Successful `SET`, `DEL`, and `EXPIRE` append text records. Startup replays WAL into memory.

Validation focus: WAL append, load, replay, invalid record handling, and no duplicate WAL write during replay.

## Stage 7

Added `Config`, `Logger`, and `Metrics`. Server and CLI gained config options. `INFO` gained command, hit/miss, connection, WAL, and storage metrics.

Validation focus: config parsing, logger behavior, metrics counters, and INFO output.

## Stage 8

Added WAL offsets and simplified master/slave replication. Slave periodically sends `SYNC offset`, replays returned WAL records, and rejects normal writes.

Validation focus: WAL offset loading, `WalReplayer`, replication state, `SYNC`, and slave read-only behavior.

## Stage 9

Added consistent hashing and client-side cluster routing. Implemented `NodeInfo`, `ConsistentHash`, `LightKVClient`, `ClusterClient`, and `lightkv_cluster_cli`.

Validation focus: stable key routing, virtual node count, remove-node behavior, empty-ring behavior, and cluster CLI smoke tests.

## Stage 10

Added `lightkv_bench`, benchmark option parsing, `make bench`, and benchmark scripts. Finalized project documentation and added resume/interview notes.

Validation focus: benchmark option parsing, benchmark tool smoke run, existing unit tests, and final documentation consistency.
