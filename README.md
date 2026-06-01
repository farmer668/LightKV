# LightKV

LightKV 是一个基于 C++17 实现的轻量级分布式 KV 缓存系统。

这是一个用于学习和展示 C++ 后端能力的 Redis-like 项目。LightKV 不依赖 Redis，只参考了 Redis 的部分核心设计思想，例如文本命令协议、TTL、LRU、WAL/AOF 风格持久化、主从复制和客户端侧分片，同时尽量保持实现小而清晰。

## 项目目标

LightKV 用来展示以下 C++ 后端工程能力：

- Linux socket 与 epoll 网络编程
- 线程安全的内存 KV 存储
- 缓存过期与淘汰策略
- WAL 持久化与重启恢复
- 基于 WAL offset 的增量主从复制
- 客户端侧一致性哈希路由
- 配置、日志、指标、测试和压测工具

## 功能特性

- C++17
- Linux socket + epoll TCP Server
- RESP-like 文本响应
- 线程安全 `KVStore`
- TTL 过期机制
- O(1) LRU 淘汰
- WAL 持久化与重启恢复
- 配置文件与命令行参数
- Logger 与 `INFO` Metrics
- 基于 WAL offset 的 master/slave 复制
- 带虚拟节点的一致性哈希环
- 单节点 `LightKVClient`
- 多节点 `ClusterClient`
- 本地 CLI、Cluster CLI 和压测工具

## 架构

```text
Client / nc / CLI / ClusterCLI
        |
        v
TcpServer / LightKVClient
        |
        v
Parser -> CommandExecutor -> KVStore
                         |-> TTL
                         |-> LRU
                         |-> WAL
                         |-> Metrics

Replication:
SlaveReplication -> SYNC offset -> Master WAL

Cluster:
ClusterClient -> ConsistentHash -> Node

Tools:
lightkv_cli / lightkv_cluster_cli / lightkv_bench
```

## 开发阶段

- Stage 0：项目骨架
- Stage 1：线程安全内存 KVStore
- Stage 2：协议解析器与本地 CLI
- Stage 3：Linux TCP epoll server
- Stage 4：TTL 过期机制
- Stage 5：LRU 淘汰机制
- Stage 6：WAL 持久化与恢复
- Stage 7：Config、Logger 和 Metrics
- Stage 8：基于 WAL offset 的 master/slave 复制
- Stage 9：一致性哈希与 ClusterClient 路由
- Stage 10：压测工具与最终文档整理

## 编译与运行

Ubuntu 下执行：

```sh
make
make test
make run
make cli
make bench
```

`make` 等价于 `make build`。

手动执行压测：

```sh
./build/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8
```

## 单节点使用示例

启动 server：

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379
```

使用 `nc` 连接后输入：

```text
PING
SET name yifei
GET name
DEL name
INFO
```

响应示例：

```text
+PONG
+OK
$5
yifei
:1
```

## TTL 示例

```text
SET token abc
EXPIRE token 10
TTL token
GET token
```

过期前，`GET token` 返回 `abc`。过期后，`GET token` 返回 `$-1`，`TTL token` 返回 `:-2`。

## LRU 示例

使用较小容量启动：

```sh
./build/lightkv_server --max-keys 2
```

然后输入：

```text
SET a 1
SET b 2
GET a
SET c 3
GET b
```

`GET a` 会刷新 `a` 的 LRU 位置，因此插入 `c` 时，最近最少使用的 `b` 可能被淘汰。

## WAL 示例

```sh
./build/lightkv_server --wal-path data/lightkv.wal
```

命令：

```text
SET persist hello
DEL old_key
EXPIRE temp 30
```

成功的 `SET`、`DEL` 和 `EXPIRE` 会追加写入 WAL。服务重启时，LightKV 会 replay WAL 来恢复内存状态。

Stage 8 之后的 WAL 记录格式：

```text
1|SET a 1
2|EXPIRE a 10
3|DEL a
```

## Config、Logger、Metrics

server 默认尝试读取：

```text
config/lightkv.example.conf
```

配置优先级：

```text
命令行参数 > 配置文件 > 默认值
```

配置示例：

```text
host=127.0.0.1
port=6379
max_keys=10000
wal_enabled=true
wal_path=data/lightkv.wal
log_level=INFO
role=master
```

`INFO` 返回 KV、WAL、复制状态和 Metrics 字段：

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
replication_offset:3
total_commands:10
hits:2
misses:1
current_connections:1
```

## 主从复制

启动 master：

```sh
./build/lightkv_server \
  --role master \
  --host 127.0.0.1 \
  --port 6379 \
  --wal-path data/master.wal
```

启动 slave：

```sh
./build/lightkv_server \
  --role slave \
  --host 127.0.0.1 \
  --port 6380 \
  --master-host 127.0.0.1 \
  --master-port 6379 \
  --wal-path data/slave.wal
```

验证：

```text
# master
SET a 1

# slave
GET a
SET b 2
```

slave 上的预期结果：

```text
$1
1
-ERR slave is read-only
```

复制逻辑是简化实现。slave 周期性发送 `SYNC offset`，接收 master 返回的新 WAL 记录，并在本地 replay，replay 过程不会再次写 WAL。

## 一致性哈希与 ClusterClient

启动三个独立节点：

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379 --wal-path data/node1.wal
./build/lightkv_server --host 127.0.0.1 --port 6380 --wal-path data/node2.wal
./build/lightkv_server --host 127.0.0.1 --port 6381 --wal-path data/node3.wal
```

启动 cluster CLI：

```sh
./build/lightkv_cluster_cli --nodes 127.0.0.1:6379,127.0.0.1:6380,127.0.0.1:6381
```

命令：

```text
ROUTE user:1
SET user:1 yifei
GET user:1
INFO
```

`ROUTE` 会显示某个 key 被路由到哪个节点：

```text
key user:1 -> node node-127.0.0.1:6379
```

一致性哈希只做客户端侧路由。LightKV 当前不做服务端集群成员感知，也不做自动数据迁移。

## 压测工具

运行：

```sh
./build/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --value-size 16 --read-ratio 0.8
```

输出字段：

```text
LightKV Bench Result
host: 127.0.0.1
port: 6379
clients: 10
requests: 1000
read_ratio: 0.8
success: 1000
failed: 0
qps: ...
avg_latency_ms: ...
p95_latency_ms: ...
p99_latency_ms: ...
```

真实压测结果应在 Ubuntu 验证后补充：

| clients | requests | read_ratio | QPS | avg latency | p95 | p99 |
| --- | --- | --- | --- | --- | --- | --- |
| 待实测 | 待实测 | 待实测 | 待实测 | 待实测 | 待实测 | 待实测 |

## 与 Redis 的关系

- LightKV 不是 Redis 的替代品。
- LightKV 不依赖 Redis。
- LightKV 是一个受 Redis 部分设计思想启发的 C++17 简化实现。
- LightKV 不支持完整 RESP、RDB、AOF rewrite、Redis Cluster slots、Sentinel、Lua、事务、模块或 Redis 兼容性保证。

## 已知限制

- value 暂不支持空格。
- WAL 暂未实现 rewrite / compaction。
- `EXPIRE` replay 使用简化的相对时间语义。
- 主从复制是最终同步，不提供强一致性保证。
- 没有自动选主。
- 没有自动故障转移。
- 一致性哈希只做客户端侧路由，不做数据迁移。
- 当前没有连接池。
- 当前没有完整 Redis 协议兼容。

## 后续计划

- 支持 RESP 子集。
- 增加 WAL rewrite / compaction。
- 增加更完善的异步写队列。
- 增加客户端连接池。
- 扩展压测场景。
- 接入 LightAgent Gateway，作为 Prompt/RAG 缓存层。

## 简历亮点

- 基于 C++17 实现 Redis-like KV 缓存，包含 Linux socket + epoll TCP Server、文本协议解析和 RESP-like 响应。
- 实现 TTL、LRU、WAL 恢复、`INFO` Metrics，以及配置和日志能力。
- 实现基于 WAL offset 的 master/slave 复制和 slave 只读保护。
- 实现一致性哈希、ClusterClient 路由，以及统计 QPS 和延迟分位数的压测工具。
