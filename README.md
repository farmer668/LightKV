# LightKV

LightKV 是一个基于 C++17 的轻量级 KV 缓存系统。

## 当前阶段

当前已完成：

- Stage 0：项目骨架
- Stage 1：线程安全内存 KVStore
- Stage 2：文本协议解析和本地 CLI
- Stage 3：Linux TCP Server + epoll

当前仍然不实现 TTL 逻辑、LRU、WAL、复制或分片。

## Stage 1 功能

- 使用 `std::unordered_map<std::string, Entry>` 存储 KV 数据。
- 使用 `std::shared_mutex` 保证并发读写安全，适合读多写少场景。
- 支持 `set`、`get`、`del`、`exists`、`size`、`clear`。
- `Entry` 预留 TTL 字段，Stage 1 暂不启用 TTL 逻辑。

## Stage 2 功能

- 新增 `Command`、`Parser`、`Response`、`CommandExecutor`。
- 支持 PING、SET、GET、DEL、EXISTS、SIZE、CLEAR、QUIT。
- 使用简化 RESP 风格响应。
- 新增本地交互式 CLI：`lightkv_cli`。
- value 暂不支持空格。

## Stage 3 功能

- Linux 下使用 socket + epoll 实现 TCP Server。
- 单线程事件循环，支持多个客户端连接。
- 使用行协议读取命令，每行一条命令，以 `\n` 结束。
- 复用 Stage 2 的 Parser + CommandExecutor + KVStore。
- 返回简化 RESP 风格响应。
- 支持 `QUIT` 返回 `+BYE` 后断开连接。
- 支持通过 `nc` 客户端测试。

## Ubuntu / Linux 构建运行

```sh
make
make run
make test
```

也可以直接运行：

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379
```

默认参数等价于：

```sh
./build/lightkv_server --host 127.0.0.1 --port 6379
```

绑定全部网卡：

```sh
./build/lightkv_server --host 0.0.0.0 --port 6379
```

运行本地 CLI：

```sh
make cli
```

## nc 测试

启动 server 后，另开一个终端：

```sh
nc 127.0.0.1 6379
```

输入：

```text
PING
SET name yifei
GET name
EXISTS name
DEL name
GET name
SIZE
QUIT
```

期望返回：

```text
+PONG
+OK
$5
yifei
:1
:1
$-1
:0
+BYE
```

## Windows 构建

Windows 下不编译 Linux-only epoll server 文件。`lightkv_server` 会提示 TCP Server 只在 Linux/Unix 可用。

```bat
scripts\build.bat
scripts\run_server.bat
scripts\test.bat
```

## CMake 构建

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

完整 Linux 验证脚本：

```sh
bash scripts/linux_verify.sh
```

## 当前限制

- 当前是单线程 epoll。
- 暂未实现 ET 模式。
- 暂未实现异步写队列。
- 暂未实现 TTL、LRU、WAL。
- value 暂不支持空格。
- Stage 4 将实现 TTL 过期机制。
