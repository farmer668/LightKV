# 开发日志

## Stage 0

- 完成 LightKV 项目骨架。
- 添加 CMake 配置、Makefile、脚本、初始文档和配置模板。

## Stage 1

- 新增 `Status`、`Entry` 和线程安全 `KVStore`。
- 支持 set、get、del、exists、size、clear。
- 新增 `test_kvstore`。

## Stage 2

- 新增 `Command`、`Parser`、`Response`、`CommandExecutor`。
- 新增本地交互式 CLI：`lightkv_cli`。
- 新增 parser 和 executor 测试。

## Stage 3

- 新增 `Buffer`，处理 TCP 半包和粘包场景。
- 新增 `TcpConnection` 和 Linux socket + epoll `TcpServer`。
- `lightkv_server` 在 Linux 下支持 TCP Server。

## Stage 4

- KVStore 增加 TTL 支持。
- 新增 `EXPIRE` / `TTL` 命令。
- 实现惰性删除和 `cleanupExpired()`。
- TCP Server 启动后启动后台过期扫描线程。
- 新增 `test_ttl`。

## Stage 5

- 新增 `LRUCache`，使用 `std::list + std::unordered_map` 维护访问顺序。
- KVStore 增加 `max_keys` 容量限制和 LRU 淘汰。
- GET 命中刷新 LRU，SET 新增或更新刷新 LRU。
- DEL、CLEAR、过期删除同步维护 LRU 状态。
- 新增 `INFO` 命令，返回 keys、max_keys、evicted_keys、expired_keys。
- lightkv_server 和 lightkv_cli 支持 `--max-keys` 参数。
- 新增 `test_lru`。
