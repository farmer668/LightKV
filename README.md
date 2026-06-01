# LightKV

LightKV is a lightweight C++17 key-value cache project built stage by stage.

## Current Stage

Completed:

- Stage 0: project scaffold
- Stage 1: thread-safe in-memory KVStore
- Stage 2: text protocol parser and local CLI
- Stage 3: Linux TCP Server based on socket + epoll
- Stage 4: TTL expiration
- Stage 5: LRU eviction
- Stage 6: WAL persistence and recovery
- Stage 7: config file, logger, and metrics
- Stage 8: WAL-offset master/slave replication
- Stage 9: consistent hashing and client-side cluster routing

## Build

On Ubuntu:

```sh
make
make test
make run
```

`make` is equivalent to `make build`.

## Configuration

The server tries to load:

```text
config/lightkv.example.conf
```

Configuration priority:

```text
command line arguments > config file > defaults
```

Example:

```sh
./build/lightkv_server --config config/lightkv.example.conf --host 0.0.0.0 --port 6379
```

Stage 8 replication settings:

```text
role=master
master_host=127.0.0.1
master_port=6379
replication_interval_ms=1000
```

Equivalent command line options:

```sh
--role master|slave
--master-host 127.0.0.1
--master-port 6379
--replication-interval-ms 1000
```

## Master/Slave Replication

Stage 8 implements simplified master/slave replication based on WAL offsets.

- Master handles normal writes and appends WAL records.
- WAL records use a monotonically increasing offset.
- Slave periodically connects to master and sends `SYNC current_offset`.
- Master returns WAL records whose offset is greater than the requested offset.
- Slave replays received WAL records directly into `KVStore`.
- Slave replay does not write local WAL again.
- Slave is read-only for normal clients and rejects `SET`, `DEL`, `EXPIRE`, and `CLEAR`.

WAL offset format:

```text
1|SET a 1
2|EXPIRE a 10
3|DEL a
```

Old Stage 6 WAL lines without an offset are read compatibly and assigned offsets by load order.

### Start Master

```sh
./build/lightkv_server \
  --role master \
  --host 127.0.0.1 \
  --port 6379 \
  --wal-path data/master.wal
```

### Start Slave

```sh
./build/lightkv_server \
  --role slave \
  --host 127.0.0.1 \
  --port 6380 \
  --master-host 127.0.0.1 \
  --master-port 6379 \
  --wal-path data/slave.wal
```

### Verify

Connect to master:

```text
SET a 1
```

Wait for slave sync, then connect to slave:

```text
GET a
SET b 2
```

Expected:

```text
$1
1
-ERR slave is read-only
```

## Client-Side Sharding

Stage 9 adds client-side sharding. LightKV servers still run as independent nodes; the client decides which node owns a key.

- `ConsistentHash` keeps a hash ring in `std::map<size_t, NodeInfo>`.
- Each real node has virtual nodes, defaulting to 100.
- Virtual node keys use `node_id#index`.
- `ClusterClient` routes `SET`, `GET`, `DEL`, `EXPIRE`, and `TTL` by key.
- `LightKVClient` wraps one short TCP request to a single LightKV server.
- `infoAll()` queries every configured node.

This stage does not implement server-side cluster membership, data migration, gossip, Redis Cluster slots, or failover.

### Start Three Nodes

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379 --wal-path data/node1.wal
./build/lightkv_server --host 127.0.0.1 --port 6380 --wal-path data/node2.wal
./build/lightkv_server --host 127.0.0.1 --port 6381 --wal-path data/node3.wal
```

### Start Cluster CLI

```sh
./build/lightkv_cluster_cli --nodes 127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381
```

Optional:

```sh
./build/lightkv_cluster_cli --nodes 127.0.0.1:6379,127.0.0.1:6380 --virtual-nodes 100
```

Example commands:

```text
ROUTE user:1
SET user:1 yifei
GET user:1
INFO
QUIT
```

`ROUTE user:1` prints the selected node:

```text
key user:1 -> node node-127.0.0.1:6379
```

## Protocol

Supported user commands:

```text
PING
SET key value
GET key
DEL key
EXISTS key
EXPIRE key seconds
TTL key
INFO
SIZE
CLEAR
QUIT
```

Internal replication command:

```text
REPLCONF ...
SYNC offset
```

`REPLCONF` is accepted as a lightweight internal handshake and returns `+OK`.
`SYNC` returns a RESP bulk string containing newline-separated WAL records after the requested offset.

## INFO

`INFO` returns a bulk string with KV, WAL, metrics, and replication status:

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
master_host:127.0.0.1
master_port:6379
replication_offset:3
last_sync_records:0
last_sync_status:none
total_syncs:0
failed_syncs:0
total_commands:10
hits:2
misses:1
```

## WAL

- `SET`, successful `DEL`, and successful `EXPIRE` append WAL.
- Startup replays WAL to restore memory state.
- `--wal-path` selects the WAL file.
- `--disable-wal` disables WAL writes and recovery.
- `EXPIRE` replay uses relative seconds from startup time.
- Values still do not support spaces.
- `CLEAR` is not persisted.
- WAL rewrite / compaction is not implemented.

## Stage 4 TTL Bugfix Note

Previously, Ubuntu TCP verification exposed a TTL bug:

```text
SET token abc
EXPIRE token 10
TTL token
GET token
```

`TTL token` could return a positive value while `GET token` incorrectly returned `$-1`. The fix centralized live-entry lookup in `KVStore` and keeps the expiration condition as:

```text
entry.has_ttl && now >= entry.expire_at
```

Now, when TTL is still positive, `GET token` returns `abc`; after expiration, `GET token` returns `$-1` and `TTL token` returns `:-2`.

## Current Limits

- No Raft.
- No automatic leader election.
- No sentinel.
- No binary replication protocol.
- No server-side cluster membership.
- No automatic data migration.
- No Redis Cluster hash slots.
- No automatic failover.
- No WAL rewrite / compaction.
- Stage 10 will focus on pressure testing and documentation cleanup.
