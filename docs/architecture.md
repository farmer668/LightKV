# Architecture

LightKV is organized as a small modular C++17 KV cache.

## Modules

- `common`: `Status`, `Config`, `Logger`, `Metrics`
- `storage`: `KVStore`, `Entry`, TTL, `LRUCache`
- `protocol`: `Command`, `Parser`, `Response`, `CommandExecutor`
- `net`: `TcpServer`, `TcpConnection`, `Buffer`
- `persistence`: `Wal`, `WalReplayer`
- `replication`: master/slave state and WAL-offset sync

## Storage

`KVStore` stores `Entry` objects in `std::unordered_map<std::string, Entry>`.

- `Entry` stores value and optional TTL metadata.
- Expiration uses lazy deletion plus a periodic cleanup worker in the server.
- `LRUCache` uses `std::list` plus `std::unordered_map` for O(1) touch/remove/evict.
- `GET` and `SET` refresh LRU order.
- `DEL`, `CLEAR`, eviction, and expiration keep LRU state in sync.

## Persistence

Stage 8 WAL lines use:

```text
offset|command
```

Example:

```text
1|SET a 1
2|EXPIRE a 10
3|DEL a
```

`Wal::open()` scans existing records to initialize `lastOffset()`. New appends use `lastOffset() + 1`.

Old Stage 6 lines without offsets are accepted by `loadRecords()` and assigned offsets according to load order.

`WalReplayer` applies `SET`, `DEL`, and `EXPIRE` directly to `KVStore`. It intentionally does not use `CommandExecutor`, so replay and slave sync do not append WAL again.

## Replication

Stage 8 implements simplified master/slave replication. It is not a consensus protocol.

Master:

- Accepts normal client writes.
- Appends successful writes to WAL.
- Handles `SYNC offset`.
- Returns all WAL records whose offset is greater than the requested offset.

Slave:

- Starts a background sync loop.
- Periodically connects to master.
- Sends `SYNC current_offset`.
- Parses returned WAL records.
- Replays them with `WalReplayer`.
- Updates replication offset and sync status.
- Rejects normal write commands with `-ERR slave is read-only`.

The sync interval defaults to 1000 ms and can be configured by `replication_interval_ms` or `--replication-interval-ms`.

## Config

Config files use simple `key=value` lines.

Priority:

```text
command line arguments > config file > defaults
```

Replication config keys:

```text
role=master
master_host=127.0.0.1
master_port=6379
replication_interval_ms=1000
```

## Logger

`Logger` is a small thread-safe singleton. It writes `[time] [LEVEL] message` to stdout.

## Metrics

`Metrics` uses atomics for command counts, hits/misses, and connections. `INFO` returns a snapshot.

## Future Work

Stage 9 will add consistent hashing and client routing. Raft, automatic election, sentinel, and binary replication are intentionally out of scope.
