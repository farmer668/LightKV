# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 模块规划

- `common`：Status、Logger、Config、Metrics
- `storage`：KVStore、Entry、TTL、LRU
- `protocol`：Command、Parser、Response
- `net`：Linux TCP Server、epoll、Connection、Buffer
- `persistence`：WAL、replay
- `replication`：master/slave、offset sync
- `cluster`：consistent hash
- `client`：LightKVClient、ClusterClient

## common

Stage 1 已实现 `Status`，用于表达操作结果。当前只保留 OK、NotFound、InvalidArgument、Error 四类状态，避免过度设计。

## storage

Stage 1 已实现线程安全的单机内存 KVStore。

- `KVStore` 使用 `std::unordered_map<std::string, Entry>` 保存数据。
- `std::shared_mutex` 用于读多写少场景：set、del、clear 使用独占锁，get、exists、size 使用共享锁。
- `Entry` 保存字符串 value，并预留 TTL 字段；TTL 逻辑会在 Stage 4 实现。
- 当前不包含 LRU、WAL、网络协议或复制逻辑。

## 后续 Linux 验证路线

平台无关的 core、storage、protocol、persistence 会优先在 Windows 和 Linux 上共同构建；Linux-only 的网络层、epoll、server runtime、replication 和 bench 会在后续 Linux 阶段实现并验证。
