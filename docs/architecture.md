# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 模块规划

- `common`：Status、Logger、Config、Metrics
- `storage`：KVStore、Entry、TTL、LRUCache
- `protocol`：Command、Parser、Response、CommandExecutor
- `net`：TcpServer、TcpConnection、Buffer
- `persistence`：WAL、replay
- `replication`：master/slave、offset sync
- `cluster`：consistent hash
- `client`：LightKVClient、ClusterClient

## storage

KVStore 是当前核心存储模块。

- 使用 `std::unordered_map<std::string, Entry>` 保存数据。
- 使用 `std::shared_mutex` 保护并发访问。
- `Entry` 保存 `value`、`has_ttl` 和 `expire_at`。
- `SET` 会覆盖 value 并清除旧 TTL。
- `GET`、`EXISTS`、`TTL` 会在访问时发现并删除已过期 key。
- `cleanupExpired()` 扫描一批 key 并删除已过期 key。

## LRU 设计

- `LRUCache` 使用 `std::list<std::string>` 保存 key 顺序，front 为最近使用，back 为最久未使用。
- `LRUCache` 使用 `std::unordered_map` 保存 key 到 list iterator 的映射。
- KVStore 在同一把锁内维护数据 map 和 LRU 状态。
- `GET` 命中刷新 LRU。
- `SET` 新 key 加入 LRU，容量超过 `max_keys` 时淘汰 LRU 尾部 key。
- `SET` 已存在 key 会更新 value、清除旧 TTL、刷新 LRU。
- `DEL`、过期删除、`CLEAR` 会同步删除 LRU 状态。
- `evicted_keys` 统计 LRU 淘汰次数，`expired_keys` 统计过期删除次数。

## protocol

- `Parser` 负责将一行文本命令解析为 `Command`。
- `CommandExecutor` 负责将 `Command` 转换为 KVStore 操作。
- `Response` 负责简化 RESP 风格编码。
- Stage 5 新增 `INFO` 命令。

## net

- `TcpServer` 负责创建监听 socket、维护 epoll 事件循环、accept 新连接、读取客户端数据并返回响应。
- `TcpConnection` 保存单个连接的 fd、peer 地址、输入缓冲和关闭状态。
- `Buffer` 负责按行提取命令，处理半包和粘包场景。
- Linux TCP Server 复用 Parser、CommandExecutor 和 KVStore。

## 后续路线

后续将进入 WAL、主从复制、一致性哈希等阶段。
