# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 模块规划

- `common`：Status、Logger、Config、Metrics
- `storage`：KVStore、Entry、TTL、LRUCache
- `protocol`：Command、Parser、Response、CommandExecutor
- `net`：TcpServer、TcpConnection、Buffer
- `persistence`：Wal、replay
- `replication`：master/slave、offset sync
- `cluster`：consistent hash
- `client`：LightKVClient、ClusterClient

## storage

- `KVStore` 使用 `unordered_map` 保存 Entry。
- `Entry` 保存 `value`、`has_ttl` 和 `expire_at`。
- `LRUCache` 使用 `std::list + std::unordered_map` 维护访问顺序。
- KVStore 在同一把锁内维护数据、TTL、LRU 和统计。

## protocol

- `Parser` 负责将文本行解析为 `Command`。
- `CommandExecutor` 负责将命令转换为 KVStore 操作。
- CommandExecutor 持有可选 `Wal*`，启用 WAL 时记录 SET、DEL、EXPIRE。
- `Response` 负责简化 RESP 风格编码。

## persistence

Stage 6 新增 WAL。

- WAL 文本记录格式：`SET key value`、`DEL key`、`EXPIRE key seconds`。
- append 成功后立即 flush。
- 启动时先 loadRecords，再按顺序 replay 到 KVStore。
- replay 期间不经过 CommandExecutor，不会重复追加 WAL。
- 非法 WAL 行会跳过并记录错误，不会导致进程崩溃。
- `EXPIRE` replay 当前使用相对时间语义，从启动时刻重新设置过期秒数。
- 当前不实现 rewrite、compaction 或二进制 WAL。

## net

- `TcpServer` 负责创建监听 socket、维护 epoll 事件循环、accept 新连接、读取客户端数据并返回响应。
- Linux TCP Server 复用 Parser、CommandExecutor、KVStore 和 WAL。

## 后续路线

Stage 7 将继续补充配置、日志、metrics 或进一步工程化能力。
