# LightKV

LightKV 是一个基于 C++17 的轻量级 KV 缓存系统。

## 当前阶段

当前已完成：

- Stage 0：项目骨架
- Stage 1：线程安全内存 KVStore
- Stage 2：文本协议解析和本地 CLI

当前仍然不实现 TCP Server、epoll、TTL 逻辑、LRU、WAL、复制或分片。

## Stage 1 功能

- 使用 `std::unordered_map<std::string, Entry>` 存储 KV 数据。
- 使用 `std::shared_mutex` 保证并发读写安全，适合读多写少场景。
- 支持 `set`、`get`、`del`、`exists`、`size`、`clear`。
- `Entry` 预留 TTL 字段，Stage 1 暂不启用 TTL 逻辑。

## Stage 2 功能

- 新增 `Command`、`Parser`、`Response`、`CommandExecutor`。
- 支持 PING、SET、GET、DEL、EXISTS、SIZE、CLEAR、QUIT。
- 使用简化 RESP 风格响应：simple string、error、integer、bulk string、null bulk string。
- 新增本地交互式 CLI：`lightkv_cli`。
- value 暂不支持空格，例如 `SET name yifei` 支持，`SET msg hello world` 暂不支持。

## CLI 示例

```sh
make
./build/lightkv_cli
```

示例输入：

```text
PING
SET name yifei
GET name
EXISTS name
DEL name
SIZE
QUIT
```

## 开发环境

- Windows + Codex：用于代码编写、核心模块构建、单元测试和 Git 提交。
- Ubuntu VMware：用于后续 Linux socket、epoll、TCP Server、nc、压测和主从复制验证。
- GitHub：用于 Windows 和 Linux 虚拟机之间同步代码。

## 后续规划

- Stage 3：Linux TCP Server + epoll
- Stage 4：TTL 过期
- Stage 5：LRU 淘汰
- Stage 6：WAL 持久化恢复
- Stage 7：配置、日志、metrics
- Stage 8：主从复制
- Stage 9：一致性哈希和客户端路由
- Stage 10：压测和文档整理

## Windows 构建

```bat
scripts\build.bat
scripts\run_server.bat
scripts\test.bat
```

## Ubuntu / Linux 构建

Ubuntu 下直接执行 `make` 即可完成 CMake configure 和 build。

```sh
make
make run
make test
```

运行 CLI：

```sh
make cli
```

也可以直接使用 CMake：

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

- 还没有 TCP Server。
- value 暂不支持空格。
- Stage 3 将实现 Linux TCP Server + epoll。
