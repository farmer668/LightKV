# LightKV Interview Q&A

## 1. Why build LightKV?

LightKV is a compact systems project for practicing C++ backend engineering. It combines networking, storage, cache policies, persistence, replication, client routing, tests, and tooling in one codebase.

## 2. How is LightKV different from Redis?

LightKV is not a Redis replacement and does not depend on Redis. It only borrows some design ideas, such as TTL, LRU, WAL-style logging, replication, and command responses. It does not implement full RESP, RDB, Redis Cluster slots, Sentinel, Lua, transactions, or Redis compatibility guarantees.

## 3. Why use epoll?

`epoll` is suitable for Linux servers with many concurrent connections because it avoids scanning all file descriptors each loop. LightKV uses epoll to handle socket readiness events in the TCP server.

## 4. How does Buffer handle partial packets and sticky packets?

The Buffer stores received bytes and extracts complete lines only when a newline is available. Partial commands remain buffered until more data arrives, and multiple commands received together can be popped one by one.

## 5. How is TTL implemented?

Each `Entry` can store `has_ttl` and `expire_at`. Operations check whether `has_ttl && now >= expire_at`. Expired keys are lazily deleted, and the server also runs periodic cleanup.

## 6. Why use list + unordered_map for LRU?

The list stores usage order and the map points keys to list iterators. This makes touch, remove, and eviction O(1). The front is most recently used and the back is least recently used.

## 7. How does WAL recover data?

Successful write commands are appended to WAL. On startup, LightKV reads records in order and replays SET, DEL, and EXPIRE into KVStore.

## 8. Why must WAL replay avoid writing WAL again?

If replay went through the normal write path and appended WAL again, restart could duplicate log records and grow the WAL incorrectly. `WalReplayer` applies records directly to KVStore.

## 9. Why use offsets in replication?

Offsets let slaves request only records they have not applied yet. A slave sends `SYNC current_offset`, and master returns WAL records whose offset is greater.

## 10. Why is slave read-only?

The simplified replication model has one write authority: the master. If slaves accepted writes, data could diverge because there is no consensus or conflict resolution.

## 11. What problem does consistent hashing solve?

It maps keys to nodes while reducing remapping when nodes are added or removed. This is useful for client-side sharding.

## 12. Why use virtual nodes?

Virtual nodes improve distribution balance. A real node appears multiple times on the ring, reducing hot spots caused by uneven hash placement.

## 13. What does INFO metrics include?

INFO includes key count, max keys, evictions, expirations, WAL state, WAL offset, replication status, command counts, hit/miss counts, and connection counts.

## 14. What are the current limits?

Values do not support spaces, WAL has no rewrite/compaction, replication is not strongly consistent, there is no failover, no server-side cluster membership, no data migration, no connection pool, and no full Redis protocol compatibility.

## 15. How would you optimize it next?

Add a RESP subset, connection pooling, async write queues, WAL compaction, better benchmark scenarios, more robust replication reconnect logic, and eventually server-side cluster membership or a stronger consensus model.

## 16. How do you test LightKV?

The project uses assert-based tests for KVStore, parser, command execution, Buffer, TTL, LRU, WAL, config, logger, metrics, replication state, consistent hash, cluster routing, and benchmark option parsing. Linux TCP and pressure tests are validated on Ubuntu.

## 17. Why not implement Raft?

Raft is outside the scope of this staged project. Stage 8 implements simplified master/slave replication based on WAL offsets, enough to demonstrate incremental sync without claiming strong consistency.
