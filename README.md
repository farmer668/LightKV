# LightKV

LightKV 是一个基于 C++17 的轻量级 KV 缓存系统。

## 当前阶段

当前已完成：

- Stage 0：项目骨架
- Stage 1：线程安全内存 KVStore
- Stage 2：文本协议解析和本地 CLI
- Stage 3：Linux TCP Server + epoll
- Stage 4：TTL 过期机制

当前仍然不实现 LRU、WAL、复制或分片。

## Stage 4 功能

- `EXPIRE key seconds`：设置 key 的过期时间。
- `TTL key`：查询 key 的剩余 TTL。
- KVStore 内部支持惰性删除，`GET`、`EXISTS`、`TTL`、`SIZE` 会处理已过期 key。
- `SET` 会清除旧 TTL，使 key 重新变为永久 key。
- Linux TCP Server 启动后会启动后台线程，每 1 秒调用 `cleanupExpired()` 清理过期 key。
- CLI 和 TCP Server 都支持 `EXPIRE` / `TTL`。

## 命令示例

本地 CLI：

```sh
make cli
```

```text
SET token abc
EXPIRE token 3
TTL token
GET token
```

等待 3 秒后：

```text
GET token
```

应返回：

```text
$-1
```

TCP Server：

```sh
make run
```

另开终端：

```sh
nc 127.0.0.1 6379
```

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
QUIT
```

## Ubuntu / Linux 构建运行

```sh
make
make test
make run
```

也可以直接运行：

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379
```

完整 Linux 验证脚本：

```sh
bash scripts/linux_verify.sh
```

## Windows 构建

Windows 下不编译 Linux-only epoll server 文件。`lightkv_server` 会提示 TCP Server 只在 Linux/Unix 可用。

```bat
scripts\build.bat
scripts\run_server.bat
scripts\test.bat
```

## 当前限制

- 当前 TCP Server 是单线程 epoll。
- 暂未实现 ET 模式。
- 暂未实现异步写队列。
- 暂未实现 LRU。
- 暂未实现 WAL。
- 暂未实现主从复制。
- value 暂不支持空格。
- Stage 5 将实现 LRU 淘汰机制。
