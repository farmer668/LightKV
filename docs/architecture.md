# Architecture

LightKV is a modular C++17 KV cache. The project is intentionally compact, but each module has a clear responsibility.

## Overview

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
ClusterClient -> ConsistentHash -> NodeInfo
```

## common

- `Status`: simple operation result type.
- `Config`: key=value config loader with typed getters.
- `Logger`: thread-safe stdout logger with DEBUG/INFO/WARN/ERROR levels.
- `Metrics`: atomic counters for commands, hits/misses, and connections.

## storage

- `Entry`: stores value plus optional TTL metadata.
- `KVStore`: thread-safe in-memory store backed by `std::unordered_map`.
- TTL: lazy deletion plus periodic cleanup in server mode.
- LRU: `LRUCache` uses `std::list` plus `std::unordered_map` for O(1) touch/remove/evict.

`KVStore` owns LRU state and keeps it synchronized during set/get/del/clear/expiration/eviction.

## protocol

- `Command`: parsed command type and arguments.
- `Parser`: line-based tokenizer and command validator.
- `Response`: RESP-like response formatter.
- `CommandExecutor`: executes commands against `KVStore`, WAL, metrics, and replication state.

Supported response types:

- `+OK` simple string
- `-ERR ...` error
- `:integer`
- `$N` bulk string
- `$-1` null bulk string

## net

- `TcpServer`: Linux socket + epoll event loop.
- `TcpConnection`: per-connection input state.
- `Buffer`: line buffer that handles partial packets and sticky packets.

Windows builds keep a scaffold path and skip Linux-only epoll code.

## persistence

- `Wal`: append-only text WAL.
- `WalRecord`: offset plus command text.
- `WalReplayer`: replays WAL commands directly into `KVStore`.

Stage 8 WAL format:

```text
1|SET a 1
2|EXPIRE a 10
3|DEL a
```

Replay does not go through `CommandExecutor`, so replay does not append WAL again.

## replication

- `ReplicationRole`: master or slave.
- `ReplicationState`: role, master address, offset, sync status, counters.
- `MasterReplication`: formats WAL records after an offset.
- `SlaveReplication`: periodically connects to master and sends `SYNC offset`.

Master accepts writes and serves `SYNC`. Slave replays WAL records and rejects normal write commands with `-ERR slave is read-only`.

## cluster

- `NodeInfo`: node id, host, port.
- `ConsistentHash`: client-side hash ring.

`ConsistentHash` uses:

- `std::hash<std::string>`
- `std::map<size_t, NodeInfo>`
- virtual node keys in the form `node_id#index`
- default 100 virtual nodes per real node

The ring is independent of server internals.

## client

- `LightKVClient`: single-node TCP client, using short connections.
- `ClusterClient`: owns a `ConsistentHash` and routes key-based commands to nodes.

`SET`, `GET`, `DEL`, `EXPIRE`, and `TTL` are routed by key. `INFO` queries all nodes.

## tools

- `lightkv_cli`: local interactive single-node CLI.
- `lightkv_cluster_cli`: client-side cluster routing CLI.
- `lightkv_bench`: single-node benchmark tool.

## Limits

- No server-side cluster membership.
- No automatic data migration.
- No Raft, Sentinel, or automatic failover.
- No complete Redis protocol compatibility.
- No WAL rewrite / compaction.
