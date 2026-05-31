# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 模块规划

- `common`：Status、Logger、Config、Metrics
- `storage`：KVStore、Entry、TTL、LRU
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
- `SIZE` 会先清理当前已过期 key，再返回数量。
- `cleanupExpired()` 扫描一批 key 并删除已过期 key，用于后台定期清理。

## TTL 设计

- `has_ttl == false` 表示永久 key。
- `has_ttl == true` 且 `now >= expire_at` 表示 key 已过期。
- 过期判断和删除集中在 KVStore 内部，Parser、CommandExecutor、TcpServer 不承载 TTL 核心逻辑。
- 后台线程和客户端请求都通过 KVStore 内部锁保护，保证线程安全。

## protocol

- `Parser` 负责将一行文本命令解析为 `Command`。
- `CommandExecutor` 负责将 `Command` 转换为 KVStore 操作。
- `Response` 负责简化 RESP 风格编码。
- Stage 4 新增 `EXPIRE` 和 `TTL`。

## net

- `TcpServer` 负责创建监听 socket、配置非阻塞 fd、维护 epoll 事件循环、accept 新连接、读取客户端数据并返回响应。
- `TcpConnection` 保存单个连接的 fd、peer 地址、输入缓冲和关闭状态。
- `Buffer` 负责按行提取命令，处理半包和粘包场景。
- Linux TCP Server 复用 Parser、CommandExecutor 和 KVStore。
- Stage 4 中 TcpServer 启动后台过期扫描线程，每 1 秒调用 KVStore 的 `cleanupExpired()`。

## 后续路线

Stage 5 将实现 LRU 淘汰机制。当前不包含 WAL、主从复制、一致性哈希或复杂定时器堆。
