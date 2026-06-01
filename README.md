# LightKV

LightKV is a lightweight distributed KV cache system written in C++17.

It is a Redis-like learning and portfolio project. It does not depend on Redis. It references several Redis ideas, such as a text command protocol, TTL, LRU, WAL/AOF-style persistence, master/slave replication, and client-side sharding, but keeps the implementation intentionally small and readable.

## Project Goals

LightKV is built to demonstrate C++ backend engineering ability:

- Linux socket and epoll network programming
- Thread-safe in-memory storage
- Cache expiration and eviction
- WAL persistence and replay
- Incremental master/slave replication
- Client-side consistent-hash routing
- Config, logging, metrics, tests, and benchmark tooling

## Features

- C++17
- Linux socket + epoll TCP server
- RESP-like text responses
- Thread-safe `KVStore`
- TTL expiration
- O(1) LRU eviction
- WAL persistence and restart recovery
- Config file and command-line options
- Logger and INFO metrics
- WAL-offset master/slave replication
- Consistent hash ring with virtual nodes
- Single-node `LightKVClient`
- Multi-node `ClusterClient`
- CLI, cluster CLI, and benchmark tool

## Architecture

```text
Client / nc / CLI / ClusterCLI
        |
        v
TcpServer / LightKVClient
        |
        v
Parser -> CommandExecutor -> KVStore
                         |-> TTL
                         |-> LRU
                         |-> WAL
                         |-> Metrics

Replication:
SlaveReplication -> SYNC offset -> Master WAL

Cluster:
ClusterClient -> ConsistentHash -> Node

Tools:
lightkv_cli / lightkv_cluster_cli / lightkv_bench
```

## Stage History

- Stage 0: project scaffold
- Stage 1: thread-safe in-memory KVStore
- Stage 2: protocol parser and local CLI
- Stage 3: Linux TCP epoll server
- Stage 4: TTL expiration
- Stage 5: LRU eviction
- Stage 6: WAL persistence and recovery
- Stage 7: Config, Logger, and Metrics
- Stage 8: WAL-offset master/slave replication
- Stage 9: consistent hash and ClusterClient routing
- Stage 10: benchmark tool and final documentation

## Build And Run

Ubuntu:

```sh
make
make test
make run
make cli
make bench
```

`make` is equivalent to `make build`.

Manual benchmark command:

```sh
./build/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8
```

## Single Node Example

Start server:

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379
```

Use `nc`:

```text
PING
SET name yifei
GET name
DEL name
INFO
```

Example responses:

```text
+PONG
+OK
$5
yifei
:1
```

## TTL Example

```text
SET token abc
EXPIRE token 10
TTL token
GET token
```

Before expiration, `GET token` returns `abc`. After expiration, `GET token` returns `$-1` and `TTL token` returns `:-2`.

## LRU Example

Start with a small capacity:

```sh
./build/lightkv_server --max-keys 2
```

Then:

```text
SET a 1
SET b 2
GET a
SET c 3
GET b
```

`GET a` refreshes `a`, so `b` is the least recently used key and can be evicted when `c` is inserted.

## WAL Example

```sh
./build/lightkv_server --wal-path data/lightkv.wal
```

Commands:

```text
SET persist hello
DEL old_key
EXPIRE temp 30
```

Successful `SET`, `DEL`, and `EXPIRE` append WAL. On restart, LightKV replays WAL to rebuild memory state.

WAL record format since Stage 8:

```text
1|SET a 1
2|EXPIRE a 10
3|DEL a
```

## Config, Logger, Metrics

Server defaults to:

```text
config/lightkv.example.conf
```

Priority:

```text
command line arguments > config file > defaults
```

Example config:

```text
host=127.0.0.1
port=6379
max_keys=10000
wal_enabled=true
wal_path=data/lightkv.wal
log_level=INFO
role=master
```

`INFO` returns KV, WAL, replication, and metrics fields:

```text
keys:2
max_keys:10000
evicted_keys:0
expired_keys:0
wal_enabled:true
wal_path:data/lightkv.wal
wal_records:3
wal_last_offset:3
role:master
replication_offset:3
total_commands:10
hits:2
misses:1
current_connections:1
```

## Master/Slave Replication

Start master:

```sh
./build/lightkv_server \
  --role master \
  --host 127.0.0.1 \
  --port 6379 \
  --wal-path data/master.wal
```

Start slave:

```sh
./build/lightkv_server \
  --role slave \
  --host 127.0.0.1 \
  --port 6380 \
  --master-host 127.0.0.1 \
  --master-port 6379 \
  --wal-path data/slave.wal
```

Verification:

```text
# master
SET a 1

# slave
GET a
SET b 2
```

Expected on slave:

```text
$1
1
-ERR slave is read-only
```

Replication is simplified. Slave periodically sends `SYNC offset`, receives newer WAL records, and replays them locally without writing WAL again.

## Consistent Hash And ClusterClient

Start three independent nodes:

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379 --wal-path data/node1.wal
./build/lightkv_server --host 127.0.0.1 --port 6380 --wal-path data/node2.wal
./build/lightkv_server --host 127.0.0.1 --port 6381 --wal-path data/node3.wal
```

Start cluster CLI:

```sh
./build/lightkv_cluster_cli --nodes 127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381
```

Commands:

```text
ROUTE user:1
SET user:1 yifei
GET user:1
INFO
```

`ROUTE` shows which node owns a key:

```text
key user:1 -> node node-127.0.0.1:6379
```

Consistent hashing is client-side only. LightKV does not perform server-side cluster membership or automatic data migration.

## Benchmark

Run:

```sh
./build/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --value-size 16 --read-ratio 0.8
```

Output fields:

```text
LightKV Bench Result
host: 127.0.0.1
port: 6379
clients: 10
requests: 1000
read_ratio: 0.8
success: 1000
failed: 0
qps: ...
avg_latency_ms: ...
p95_latency_ms: ...
p99_latency_ms: ...
```

Real benchmark results should be filled in after Ubuntu validation:

| clients | requests | read_ratio | QPS | avg latency | p95 | p99 |
| --- | --- | --- | --- | --- | --- | --- |
| To be measured | To be measured | To be measured | To be measured | To be measured | To be measured | To be measured |

## Relationship With Redis

- LightKV is not a Redis replacement.
- LightKV does not depend on Redis.
- It is a simplified C++17 implementation inspired by several Redis design ideas.
- It does not support full RESP, RDB, AOF rewrite, Redis Cluster slots, Sentinel, Lua, transactions, modules, or Redis compatibility guarantees.

## Known Limits

- Values do not support spaces.
- WAL rewrite / compaction is not implemented.
- `EXPIRE` replay uses simplified relative time.
- Master/slave replication is eventually synchronized and does not provide strong consistency.
- No automatic leader election.
- No automatic failover.
- Consistent hashing is client-side routing only and does not migrate data.
- No connection pool yet.
- No full Redis protocol compatibility.

## Future Work

- Support a RESP subset.
- Add WAL rewrite / compaction.
- Add a better async write queue.
- Add client connection pooling.
- Expand benchmark scenarios.
- Integrate with LightAgent Gateway as a Prompt/RAG cache layer.

## Resume Highlights

- Built a C++17 Redis-like KV cache with Linux socket + epoll TCP server, text protocol parsing, and RESP-like responses.
- Implemented TTL, LRU, WAL recovery, INFO metrics, and config/logging support.
- Implemented WAL-offset master/slave replication and slave read-only enforcement.
- Implemented consistent hashing, ClusterClient routing, and a benchmark tool with QPS and latency percentiles.
