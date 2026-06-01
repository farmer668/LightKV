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

当前仍然不实现主从复制或一致性哈希。

## Stage 6 功能

- 新增文本 WAL，默认 server 路径为 `data/lightkv.wal`。
- `SET`、成功 `DEL`、成功 `EXPIRE` 会写入 WAL。
- 启动时 replay WAL，恢复内存 KV 状态。
- 支持 `--wal-path` 指定 WAL 文件。
- 支持 `--disable-wal` 关闭 WAL。
- `INFO` 输出 WAL 状态：`wal_enabled`、`wal_path`、`wal_records`。
- `lightkv_cli` 默认使用 `data/lightkv_cli.wal`，避免和 server WAL 冲突。

## 运行参数

Linux TCP Server：

```sh
make
./build/lightkv_server --host 127.0.0.1 --port 6379 --max-keys 10000 --wal-path data/lightkv.wal
```

关闭 WAL：

```sh
./build/lightkv_server --disable-wal
```

本地 CLI：

```sh
make cli
./build/lightkv_cli --max-keys 10000 --wal-path data/lightkv_cli.wal
```

## WAL 示例

启动 server：

```sh
make run
```

另开终端：

```sh
nc 127.0.0.1 6379
```

输入：

```text
SET persist hello
QUIT
```

停止 server 后重新启动，再查询：

```text
GET persist
```

应返回：

```text
$5
hello
```

## INFO

`INFO` 返回简化 bulk string，内容类似：

```text
keys:3
max_keys:10000
evicted_keys:0
expired_keys:0
wal_enabled:true
wal_path:data/lightkv.wal
wal_records:3
```

## WAL 限制

- WAL 文本格式为 `SET key value`、`DEL key`、`EXPIRE key seconds`。
- value 暂不支持空格。
- `EXPIRE` replay 使用相对时间简化，重启后从当前时间重新设置过期秒数。
- 暂未实现 AOF rewrite / compaction。
- 暂未实现二进制日志。
- `CLEAR` 当前不参与 WAL 持久化。

## Stage 4 TTL Bugfix

复现：

```text
SET token abc
EXPIRE token 10
TTL token
GET token
```

错误现象：`TTL token` 仍为正数时，`GET token` 错误返回 `$-1`。

修复：修正 KVStore 中 GET/TTL 的过期判断路径，统一通过 live-entry helper 判断 key 是否真正过期，保证未过期时 `GET` 返回 value。

验证：TTL 为正数时 `GET token` 返回 `abc`；过期后 `GET token` 返回 `$-1`，`TTL token` 返回 `:-2`。

## 当前限制

- 暂未实现主从复制。
- 暂未实现一致性哈希。
- value 暂不支持空格。
- Stage 7 将实现配置、日志、metrics 或进一步工程化能力。
