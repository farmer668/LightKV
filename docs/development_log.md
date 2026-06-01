# 开发日志

## Stage 0

- 完成 LightKV 项目骨架。

## Stage 1

- 新增 `Status`、`Entry` 和线程安全 `KVStore`。
- 支持 set、get、del、exists、size、clear。

## Stage 2

- 新增 `Command`、`Parser`、`Response`、`CommandExecutor`。
- 新增本地交互式 CLI：`lightkv_cli`。

## Stage 3

- 新增 `Buffer`、`TcpConnection` 和 Linux socket + epoll `TcpServer`。

## Stage 4

- KVStore 增加 TTL 支持。
- 新增 `EXPIRE` / `TTL` 命令。
- 实现惰性删除和 `cleanupExpired()`。

## Stage 5

- 新增 `LRUCache`。
- KVStore 增加 `max_keys` 容量限制和 LRU 淘汰。
- 新增 `INFO` 命令。

## Stage 6

- 新增 `Wal`。
- SET、成功 DEL、成功 EXPIRE 写入 WAL。
- lightkv_server 和 lightkv_cli 启动时 replay WAL。

## Stage 7

- 新增 `Config`，支持 key=value 配置文件、注释、空行、trim 和 string/int/size_t/bool 类型读取。
- 新增 `Logger`，支持 DEBUG、INFO、WARN、ERROR 日志级别。
- 新增 `Metrics`，统计命令数、hits/misses 和连接数。
- lightkv_server 默认尝试读取 `config/lightkv.example.conf`，并支持 `--config`。
- lightkv_cli 支持 `--config`。
- 命令行参数优先级高于配置文件。
- INFO 输出增加 Metrics 字段。
- 新增 `test_config`、`test_logger`、`test_metrics`。
