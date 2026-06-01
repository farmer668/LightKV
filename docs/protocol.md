# Protocol

LightKV uses a simple line-based text protocol with RESP-style responses.

## User Commands

| Command | Args | Example |
| --- | --- | --- |
| PING | 0 | `PING` |
| SET | 2 | `SET name yifei` |
| GET | 1 | `GET name` |
| DEL | 1 | `DEL name` |
| EXISTS | 1 | `EXISTS name` |
| EXPIRE | 2 | `EXPIRE name 10` |
| TTL | 1 | `TTL name` |
| INFO | 0 | `INFO` |
| SIZE | 0 | `SIZE` |
| CLEAR | 0 | `CLEAR` |
| QUIT | 0 | `QUIT` |

Values do not support spaces in the current text protocol.

## Internal Replication Command

```text
REPLCONF ...
SYNC offset
```

`REPLCONF` is accepted as a lightweight internal handshake and returns `+OK`.
`SYNC` is intended for slave replication.

- Master returns WAL records with `record.offset > offset`.
- Slave returns `-ERR only master can sync`.
- `REPLCONF` and `SYNC` do not write WAL.

Response is a bulk string. The body is newline-separated WAL records:

```text
1|SET a 1
2|SET b 2
3|EXPIRE b 10
```

If there are no newer records, the body is empty.

## Slave Read-Only Rule

When `role=slave`, normal client write commands are rejected:

```text
-ERR slave is read-only
```

Rejected commands:

- `SET`
- `DEL`
- `EXPIRE`
- `CLEAR`

Read commands are still allowed:

- `PING`
- `GET`
- `EXISTS`
- `SIZE`
- `TTL`
- `INFO`
- `QUIT`

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
