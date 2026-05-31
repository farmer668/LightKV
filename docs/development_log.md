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
