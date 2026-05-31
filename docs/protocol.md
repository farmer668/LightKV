# 协议设计

Stage 2 实现本地文本命令协议，Stage 3 将同一协议接入 Linux TCP Server。当前不是完整 Redis 协议，只使用简化 RESP 风格响应。

## TCP 使用方式

- 客户端通过 TCP 连接 `127.0.0.1:6379`。
- 每行一个命令，以 `\n` 作为命令结束。
- 服务端按行解析命令，返回 RESP-like 响应。
- 发送 `QUIT` 后，服务端返回 `+BYE\r\n` 并断开连接。

```sh
nc 127.0.0.1 6379
```

## 解析规则

- 按空白字符分割输入。
- 命令大小写不敏感。
- 参数大小写保持原样。
- value 暂不支持空格。
- 空行返回 Invalid，错误信息为 `empty command`。
- 未知命令返回 Invalid。
- 参数数量错误返回 Invalid。

## 支持命令

| 命令 | 参数数量 | 示例 |
| --- | --- | --- |
| PING | 0 | `PING` |
| SET | 2 | `SET name yifei` |
| GET | 1 | `GET name` |
| DEL | 1 | `DEL name` |
| EXISTS | 1 | `EXISTS name` |
| SIZE | 0 | `SIZE` |
| CLEAR | 0 | `CLEAR` |
| QUIT | 0 | `QUIT` |

## 响应格式

- Simple string：`+OK\r\n`
- Error：`-ERR message\r\n`
- Integer：`:1\r\n`
- Bulk string：`$5\r\nyifei\r\n`
- Null bulk string：`$-1\r\n`

## 命令语义

- `PING`：返回 `+PONG\r\n`
- `SET key value`：写入 key，成功返回 `+OK\r\n`
- `GET key`：存在时返回 bulk string，不存在时返回 `$-1\r\n`
- `DEL key`：删除成功返回 `:1\r\n`，key 不存在返回 `:0\r\n`
- `EXISTS key`：存在返回 `:1\r\n`，不存在返回 `:0\r\n`
- `SIZE`：返回当前 key 数量
- `CLEAR`：清空 KVStore，返回 `+OK\r\n`
- `QUIT`：返回 `+BYE\r\n`
- Invalid：返回 `-ERR message\r\n`

## nc 示例

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

期望响应：

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
