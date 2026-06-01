# LightKV

LightKV 是一个基于 C++17 的轻量级 KV 缓存系统。

## 当前阶段

当前已完成：

- Stage 0：项目骨架
- Stage 1：线程安全内存 KVStore
- Stage 2：文本协议解析和本地 CLI
- Stage 3：Linux TCP Server + epoll
- Stage 4：TTL 过期机制
- Stage 5：LRU 淘汰机制
- Stage 6：WAL 持久化与重启恢复
- Stage 7：配置文件、日志系统和 Metrics

## 配置

server 默认尝试读取：

```text
config/lightkv.example.conf
```

如果配置文件不存在，则使用内置默认值。命令行参数优先级高于配置文件：

```text
命令行参数 > 配置文件 > 默认值
```

启动示例：

```sh
./build/lightkv_server --config config/lightkv.example.conf
```

命令行覆盖配置：

```sh
./build/lightkv_server --config config/lightkv.example.conf --host 0.0.0.0 --port 6380 --max-keys 20000
```

CLI 也支持配置文件：

```sh
./build/lightkv_cli --config config/lightkv.example.conf --wal-path data/lightkv_cli.wal
```

## 日志

支持日志级别：

- DEBUG
- INFO
- WARN
- ERROR

配置文件：

```text
log_level=INFO
```

命令行：

```sh
./build/lightkv_server --log-level WARN
```

## Metrics

`INFO` 命令返回运行指标，包含：

- KV/LRU/TTL：`keys`、`max_keys`、`evicted_keys`、`expired_keys`
- WAL：`wal_enabled`、`wal_path`、`wal_records`
- 命令计数：`total_commands`、`get_commands`、`set_commands`、`del_commands`、`expire_commands`、`ttl_commands`、`info_commands`
- 命中统计：`hits`、`misses`
- 连接统计：`connected_clients`、`total_connections`、`current_connections`

示例：

```text
INFO
```

返回 bulk string，内容类似：

```text
keys:2
max_keys:10000
evicted_keys:0
expired_keys:0
wal_enabled:true
wal_path:data/lightkv.wal
wal_records:3
total_commands:10
get_commands:3
set_commands:2
del_commands:1
expire_commands:1
ttl_commands:1
info_commands:1
hits:2
misses:1
total_connections:1
current_connections:1
```

## WAL

- `SET`、成功 `DEL`、成功 `EXPIRE` 会写入 WAL。
- 启动时 replay WAL 恢复内存 KV 状态。
- 支持 `--wal-path`。
- 支持 `--disable-wal`。
- `EXPIRE` replay 使用相对时间简化。
- 暂未实现 WAL rewrite / compaction。

## 当前限制

- 暂未实现主从复制。
- 暂未实现一致性哈希。
- 暂未实现 WAL rewrite / compaction。
- value 暂不支持空格。
- Stage 8 将实现主从复制。
