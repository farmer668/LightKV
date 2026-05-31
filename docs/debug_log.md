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
