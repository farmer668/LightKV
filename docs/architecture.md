# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 未来模块规划

- `common`：Status、Logger、Config、Metrics
- `storage`：KVStore、Entry、TTL、LRU
- `protocol`：Command、Parser、Response
- `net`：Linux TCP Server、epoll、Connection、Buffer
- `persistence`：WAL、replay
- `replication`：master/slave、offset sync
- `cluster`：consistent hash
- `client`：LightKVClient、ClusterClient

## Stage 0 范围

Stage 0 只提供可构建的项目骨架。后续会优先补充平台无关的 core、storage、protocol、persistence 等模块；Linux-only 的网络层、epoll、server runtime、replication 和 bench 会在后续 Linux 阶段实现并验证。
