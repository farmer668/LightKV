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

## common

Stage 1 已实现 `Status`，用于表达操作结果。当前只保留 OK、NotFound、InvalidArgument、Error 四类状态，避免过度设计。

## storage

Stage 1 已实现线程安全的单机内存 KVStore。

- `KVStore` 使用 `std::unordered_map<std::string, Entry>` 保存数据。
- `std::shared_mutex` 用于读多写少场景：set、del、clear 使用独占锁，get、exists、size 使用共享锁。
- `Entry` 保存字符串 value，并预留 TTL 字段；TTL 逻辑会在 Stage 4 实现。
- 当前不包含 LRU、WAL 或复制逻辑。

## protocol

Stage 2 已实现本地文本协议模块。

- `Parser` 负责将一行文本命令解析为 `Command`。
- `Command` 保存命令类型、参数、原始输入和解析错误。
- `CommandExecutor` 负责将 `Command` 转换为 KVStore 操作。
- `Response` 负责简化 RESP 风格编码。
- `lightkv_cli` 复用 Parser、CommandExecutor 和 KVStore，当前只操作本地内存。

## net

Stage 3 已实现 Linux-only TCP Server。

- `TcpServer` 负责创建监听 socket、配置非阻塞 fd、维护 epoll 事件循环、accept 新连接、读取客户端数据并返回响应。
- `TcpConnection` 保存单个连接的 fd、peer 地址、输入缓冲和关闭状态。
- `Buffer` 负责按行提取命令，处理半包和粘包场景。
- 网络层复用已有 Parser、CommandExecutor 和 KVStore，不在连接对象中写业务逻辑。
- 当前为单线程 epoll，不包含线程池、异步写队列、Timer、TTL、LRU 或 WAL。

## 后续 Linux 验证路线

平台无关的 core、storage、protocol、persistence 会继续在 Windows 和 Linux 上共同构建；Linux-only 的网络层、epoll、server runtime、replication 和 bench 会在 Linux 阶段实现并验证。
