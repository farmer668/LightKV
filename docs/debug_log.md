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
