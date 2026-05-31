# LightKV

LightKV 是一个基于 C++17 的轻量级 KV 缓存系统。

## Stage 0

Stage 0 只完成项目初始化和工程骨架，不实现 KV 存储、TCP Server、epoll、Redis 协议、持久化、复制或分片等复杂业务逻辑。

## 开发环境

- Windows + Codex：用于代码编写、核心模块构建、单元测试和 Git 提交。
- Ubuntu VMware：用于后续 Linux socket、epoll、TCP Server、nc、压测和主从复制验证。
- GitHub：用于 Windows 和 Linux 虚拟机之间同步代码。

## 后续规划

- Stage 1：单机线程安全 KVStore
- Stage 2：文本协议解析和本地 CLI
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

## Linux 验证

```sh
bash scripts/linux_verify.sh
```

## CMake 构建

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Make 构建

```sh
make build
make run
make test
make clean
```
