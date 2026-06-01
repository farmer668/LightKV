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

当前仍然不实现 WAL、主从复制或一致性哈希。

## Stage 5 功能

- KVStore 支持 `max_keys` 容量限制，默认 `10000`。
- `max_keys == 0` 表示不限制容量。
- 使用 `std::list + std::unordered_map` 实现 O(1) LRU。
- `GET` 命中时刷新 LRU。
- `SET` 新 key 时加入 LRU；容量超限时淘汰最近最少使用的 key。
- `SET` 已存在 key 时更新 value、清除旧 TTL、刷新 LRU。
- `DEL`、过期删除、`CLEAR` 会同步维护 LRU 状态。
- `INFO` 命令返回 `keys`、`max_keys`、`evicted_keys`、`expired_keys`。

## 运行参数

Linux TCP Server：

```sh
make
./build/lightkv_server --host 127.0.0.1 --port 6379 --max-keys 10000
```

本地 CLI：

```sh
make cli
./build/lightkv_cli --max-keys 10000
```

## 命令示例

```text
SET a 1
SET b 2
GET a
SET c 3
INFO
```

当 `--max-keys 2` 时，`GET a` 会刷新 `a`，随后写入 `c` 会淘汰更久未使用的 `b`。

## INFO

`INFO` 返回简化 bulk string，内容类似：

```text
keys:3
max_keys:10000
evicted_keys:0
expired_keys:0
```

## TTL 示例

```text
SET token abc
EXPIRE token 3
TTL token
GET token
```

等待 3 秒后：

```text
GET token
TTL token
```

应返回 `$-1` 和 `:-2`。

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

- 暂未实现 WAL。
- 暂未实现主从复制。
- 暂未实现一致性哈希。
- value 暂不支持空格。
