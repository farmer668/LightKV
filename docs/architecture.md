# 架构设计

LightKV 规划为一个模块化的 C++17 KV 缓存系统。

## 模块规划

- `common`：Status、Config、Logger、Metrics
- `storage`：KVStore、Entry、TTL、LRUCache
- `protocol`：Command、Parser、Response、CommandExecutor
- `net`：TcpServer、TcpConnection、Buffer
- `persistence`：Wal、replay
- `replication`：master/slave、offset sync
- `cluster`：consistent hash
- `client`：LightKVClient、ClusterClient

## Config

- 配置文件格式为简单 `key=value`。
- 忽略空行和 `#` 开头注释。
- key 和 value 两侧会 trim。
- 支持 string、int、size_t、bool 类型读取。
- bool 支持 true/false/1/0/yes/no。
- 配置文件不存在时使用默认值。
- 参数优先级为：命令行参数 > 配置文件 > 默认值。

## Logger

- Logger 是简单单例日志器。
- 支持 DEBUG、INFO、WARN、ERROR。
- 输出到 stdout，格式为 `[time] [LEVEL] message`。
- TcpServer 启动、WAL replay、连接建立/断开和错误路径使用 Logger。

## Metrics

- Metrics 使用 `std::atomic<size_t>` 保存计数。
- CommandExecutor 更新命令计数。
- GET 命中时 hits +1，未命中时 misses +1。
- TcpServer accept 新连接时增加 total/current connections。
- TcpServer close connection 时减少 current connections。
- INFO 输出 Metrics 快照。

## persistence

- WAL 文本记录格式：`SET key value`、`DEL key`、`EXPIRE key seconds`。
- replay 期间不经过 CommandExecutor，不会重复追加 WAL。
- `EXPIRE` replay 当前使用相对时间。

## 后续路线

Stage 8 将实现主从复制。
