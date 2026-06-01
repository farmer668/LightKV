# Protocol

LightKV uses a simple line-based text protocol with RESP-like responses. It is not a complete Redis protocol implementation.

## Normal Commands

| Command | Args | Example |
| --- | --- | --- |
| PING | 0 | `PING` |
| SET | 2 | `SET key value` |
| GET | 1 | `GET key` |
| DEL | 1 | `DEL key` |
| EXISTS | 1 | `EXISTS key` |
| SIZE | 0 | `SIZE` |
| CLEAR | 0 | `CLEAR` |
| EXPIRE | 2 | `EXPIRE key seconds` |
| TTL | 1 | `TTL key` |
| INFO | 0 | `INFO` |
| QUIT | 0 | `QUIT` |

Values currently do not support spaces.

## Internal Replication Commands

```text
REPLCONF ...
SYNC offset
```

- `REPLCONF` is a lightweight handshake and returns `+OK`.
- `SYNC offset` returns WAL records whose offset is greater than `offset`.
- Slave nodes return `-ERR only master can sync` for `SYNC`.
- Replication commands do not write WAL.

`SYNC` returns a bulk string body:

```text
1|SET a 1
2|SET b 2
3|EXPIRE b 10
```

## Cluster CLI Commands

`lightkv_cluster_cli` is a client-side routing tool.

| Command | Args | Behavior |
| --- | --- | --- |
| SET | 2 | Route key and send `SET key value` |
| GET | 1 | Route key and send `GET key` |
| DEL | 1 | Route key and send `DEL key` |
| EXPIRE | 2 | Route key and send `EXPIRE key seconds` |
| TTL | 1 | Route key and send `TTL key` |
| INFO | 0 | Query all configured nodes |
| ROUTE | 1 | Print the selected node for a key |
| QUIT | 0 | Exit |

Example:

```text
ROUTE user:1
key user:1 -> node node-127.0.0.1:6379
```

## Response Formats

Simple string:

```text
+OK
```

Error:

```text
-ERR message
```

Integer:

```text
:1
```

Bulk string:

```text
$5
yifei
```

Null bulk string:

```text
$-1
```

## INFO Fields

KV/LRU/TTL:

- `keys`
- `max_keys`
- `evicted_keys`
- `expired_keys`

WAL:

- `wal_enabled`
- `wal_path`
- `wal_records`
- `wal_last_offset`

Replication:

- `role`
- `master_host`
- `master_port`
- `replication_offset`
- `last_sync_records`
- `last_sync_status`
- `total_syncs`
- `failed_syncs`

Metrics:

- `total_commands`
- `ping_commands`
- `get_commands`
- `set_commands`
- `del_commands`
- `exists_commands`
- `expire_commands`
- `ttl_commands`
- `info_commands`
- `hits`
- `misses`
- `connected_clients`
- `total_connections`
- `current_connections`

## WAL Behavior

- `SET key value` writes WAL after success.
- `DEL key` writes WAL only when a key is actually deleted.
- `EXPIRE key seconds` writes WAL after success.
- `GET`, `EXISTS`, `TTL`, `SIZE`, `INFO`, `CLEAR`, `REPLCONF`, and `SYNC` do not write WAL.
- `CLEAR` is not persisted in the current stage.
