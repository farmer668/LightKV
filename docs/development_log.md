# 开发日志

## Stage 0

- 完成 LightKV 项目骨架。
- 添加 CMake 配置，生成 `lightkv_server`。
- 添加 Makefile，提供 build、run、test、clean 目标。
- 添加 Windows bat 脚本：build、run_server、test。
- 添加 Linux/Git Bash shell 脚本：build、run_server、linux_verify。
- 添加初始文档、配置模板和 Git ignore 规则。

## Stage 1

- 新增 `Status`，用于表达基础操作结果。
- 新增 `Entry`，保存 value 并预留 TTL 字段。
- 新增线程安全 `KVStore`，支持 set、get、del、exists、size、clear。
- 新增 `test_kvstore`，使用标准库 `assert` 覆盖基础 KV 操作。
- 修改 CMake，将 Stage 1 核心源码加入 `lightkv_server`，并注册 `test_kvstore`。
- 修改 Makefile，使直接执行 `make` 默认等价于 `make build`。

## Stage 2

- 新增协议解析模块：`Command`、`Parser`、`Response`、`CommandExecutor`。
- 新增简化 RESP 风格 Response 编码。
- 新增 `CommandExecutor`，将文本命令映射到 KVStore 操作。
- 新增本地交互式 CLI：`lightkv_cli`。
- 新增 parser 和 executor 测试：`test_parser`、`test_command_executor`。
- 修改 `lightkv_server`，增加协议层轻量自检。

## Stage 3

- 新增 `Buffer`，处理 TCP 半包和粘包场景。
- 新增 `TcpConnection`，保存单个客户端连接状态。
- 新增 `TcpServer`，在 Linux 下基于 socket + epoll 实现 TCP Server。
- `lightkv_server` 在 Linux 下支持启动 TCP Server。
- TCP Server 复用 Parser、CommandExecutor 和 KVStore。
- 支持通过 `nc 127.0.0.1 6379` 进行文本协议测试。

## Stage 4

- KVStore 增加 TTL 支持。
- 新增 `EXPIRE` / `TTL` 命令。
- 实现 get、exists、ttl、size 的惰性删除。
- 实现 `cleanupExpired()`，用于批量清理已过期 key。
- TCP Server 启动后启动后台过期扫描线程。
- 新增 `test_ttl` 覆盖 TTL 核心语义。
