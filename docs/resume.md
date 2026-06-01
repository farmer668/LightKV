# LightKV Resume Notes

## Project Title

LightKV: C++17 lightweight distributed KV cache system

## One-Sentence Description

Built a Redis-like KV cache in C++17 with Linux epoll networking, TTL/LRU cache management, WAL recovery, master/slave replication, client-side sharding, and benchmark tooling.

## Resume Bullets

- Implemented a lightweight C++17 KV cache with a custom TCP text protocol, Linux socket + epoll event loop, multi-client access, and RESP-like response formatting.
- Designed TTL expiration and LRU eviction on top of an unordered_map-based KVStore, using std::list + unordered_map for O(1) LRU refresh and eviction, with INFO metrics for hits, misses, evictions, and expirations.
- Built WAL persistence and recovery for SET/DEL/EXPIRE, replaying logs on restart and extending WAL records with offsets for simplified master/slave incremental replication.
- Implemented consistent hashing and ClusterClient routing for multi-node key sharding, plus a benchmark tool reporting QPS, success/failure counts, average latency, P95, and P99.

## One-Minute Interview Pitch

LightKV is a C++17 Redis-like KV cache I built to practice backend systems engineering. It has a Linux epoll TCP server, a small text protocol, a thread-safe in-memory KVStore, TTL expiration, LRU eviction, WAL persistence, INFO metrics, and config/logging support. I also added simplified WAL-offset master/slave replication, where slaves periodically fetch incremental WAL records and replay them locally, and client-side sharding using consistent hashing and virtual nodes. The project includes CLI tools, a cluster CLI, assert-based unit tests, and a benchmark tool that reports QPS and latency percentiles. It is not intended to replace Redis; it is a compact implementation that demonstrates networking, storage, caching, replication, routing, and engineering workflow.
