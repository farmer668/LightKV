# 调试日志

暂无重大问题。

## Stage 1

- Windows 当前环境缺少 `cmake`、`ctest`、`make`，完整构建测试留给 Ubuntu 虚拟机执行。
- 已使用 Windows 可用的 `g++` 做基础编译和运行验证。

## Stage 2

- Windows 当前环境仍缺少 `cmake`、`ctest`、`make`。
- 已使用 Windows 可用的 `g++` 直接编译并运行 `lightkv_server`、`test_parser`、`test_command_executor`。

## Stage 3

- Windows 当前环境仍缺少 `cmake`、`ctest`、`make`。
- Linux-only 的 `TcpServer.cpp` 使用 epoll/socket 头文件，Windows 下通过 CMake 平台判断跳过编译。
- 已使用 Windows 可用的 `g++` 验证 Windows scaffold server、`test_buffer` 和既有平台无关测试。

## Stage 4

- Windows 当前环境仍缺少 `cmake`、`ctest`、`make`。
- Linux-only TCP Server 完整验证留给 Ubuntu 环境。
- 已使用 Windows 可用的 `g++` 验证 TTL 核心、协议命令和平台无关测试。

## Stage 4 bugfix: GET returned null before TTL expired

- 复现命令：`PING`、`SET token abc`、`EXPIRE token 10`、`TTL token`、`GET token`。
- 错误现象：Ubuntu TCP 验证中 `TTL token` 仍返回正数，但紧随其后的 `GET token` 返回 `$-1`。
- 根因：TTL 访问路径缺少专门覆盖 “TTL 未过期时 GET 必须返回 value” 的回归测试，过期删除判断在多个方法中分散，容易误删设置了 TTL 但尚未过期的 key。
- 修复方式：将 KVStore 中“如果已过期则删除”的逻辑收敛到 `eraseIfExpiredLocked()`，并确保只有 `has_ttl == true && now >= expire_at` 时才删除。
- 验证方式：新增 TTL 与 CommandExecutor 回归测试，覆盖 TTL 正数时 GET 返回 bulk string、过期后 GET 返回 null、cleanupExpired 不删除未过期 key。

## Stage 4 bugfix: GET returned null while TTL was still positive

- 复现命令：`SET token abc`、`EXPIRE token 10`、`TTL token`、`GET token`。
- 错误现象：`TTL token` 返回正数，例如 `:6`，但 `GET token` 返回 `$-1`。
- 根因：代码中没有 const `KVStore::get` 重载，`CommandExecutor` 调用的是唯一的非 const `KVStore::get`。问题集中在 KVStore 的 live-entry 访问路径不够集中，`get`、`exists`、`ttl`、`expire` 分别做查找和过期删除，缺少一个共享入口来保证 “has_ttl 为 true 但 now < expire_at” 仍然是有效 key。
- 修复方式：新增 `findLiveEntryLocked()`，让 `get`、`exists`、`ttl`、`expire` 统一通过同一个 helper 查找 key 并仅在 `has_ttl == true && now >= expire_at` 时删除；新增 `test_ttl_regression` 覆盖 `TTL` 仍有效时 `GET` 必须返回 `abc`。
- 验证方式：Ubuntu 可执行 `make clean`、`make`、`make test`、`make run`，再用 `nc 127.0.0.1 6379` 输入上述复现命令，期望 `GET token` 返回 `$3\r\nabc\r\n`。

## Stage 5

- 暂无重大问题。
