# Development Log

## Stage 0

- Created the LightKV project scaffold.

## Stage 1

- Added `Status`, `Entry`, and thread-safe `KVStore`.
- Supported set/get/del/exists/size/clear.

## Stage 2

- Added `Command`, `Parser`, `Response`, and `CommandExecutor`.
- Added local interactive `lightkv_cli`.

## Stage 3

- Added `Buffer`, `TcpConnection`, and Linux socket + epoll `TcpServer`.

## Stage 4

- Added TTL support to `KVStore`.
- Added `EXPIRE` and `TTL`.
- Implemented lazy expiration and `cleanupExpired()`.

## Stage 5

- Added `LRUCache`.
- Added `max_keys` and O(1) LRU eviction to `KVStore`.
- Added `INFO`.

## Stage 6

- Added `Wal`.
- Wrote `SET`, successful `DEL`, and successful `EXPIRE` to WAL.
- Replayed WAL during `lightkv_server` and `lightkv_cli` startup.

## Stage 7

- Added `Config` for key=value config files.
- Added `Logger` with DEBUG/INFO/WARN/ERROR levels.
- Added `Metrics` for command counts, hits/misses, and connection counts.
- Added config support to server and CLI.
- Added metrics fields to `INFO`.
- Added `test_config`, `test_logger`, and `test_metrics`.

## Stage 8

- Changed WAL format to `offset|command`.
- Added compatible loading for old WAL records without offsets.
- Added `WalRecord`, `loadRecordsAfter(offset)`, and `lastOffset()`.
- Added `WalReplayer` so replay does not go through `CommandExecutor` and does not append WAL again.
- Added replication state: role, master address, replication offset, sync status, and sync counters.
- Added internal `REPLCONF` handshake and `SYNC offset` command for master-side incremental WAL fetch.
- Added slave read-only enforcement for `SET`, `DEL`, `EXPIRE`, and `CLEAR`.
- Added Linux/Unix slave background sync loop.
- Added replication fields to `INFO`.
- Added tests: `test_wal_offset`, `test_wal_replayer`, and `test_replication_state`.
