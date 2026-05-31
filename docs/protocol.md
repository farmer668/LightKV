# 协议设计

LightKV 当前支持本地 CLI 和 Linux TCP Server 的文本命令协议。当前不是完整 Redis 协议，只使用简化 RESP 风格响应。

## TCP 使用方式

- 客户端通过 TCP 连接 `127.0.0.1:6379`。
- 每行一个命令，以 `\n` 作为命令结束。
- 服务端按行解析命令，返回 RESP-like 响应。
- 发送 `QUIT` 后，服务端返回 `+BYE\r\n` 并断开连接。

```sh
nc 127.0.0.1 6379
```

## 支持命令

| 命令 | 参数数量 | 示例 |
| --- | --- | --- |
| PING | 0 | `PING` |
| SET | 2 | `SET name yifei` |
| GET | 1 | `GET name` |
| DEL | 1 | `DEL name` |
| EXISTS | 1 | `EXISTS name` |
| EXPIRE | 2 | `EXPIRE name 10` |
| TTL | 1 | `TTL name` |
| SIZE | 0 | `SIZE` |
| CLEAR | 0 | `CLEAR` |
| QUIT | 0 | `QUIT` |

## 响应格式

- Simple string：`+OK\r\n`
- Error：`-ERR message\r\n`
- Integer：`:1\r\n`
- Bulk string：`$5\r\nyifei\r\n`
- Null bulk string：`$-1\r\n`

## TTL 语义

`EXPIRE key seconds`：

- key 存在且设置成功，返回 `:1\r\n`
- key 不存在，返回 `:0\r\n`
- seconds <= 0 时，如果 key 存在则删除并返回 `:1\r\n`
- seconds 不是合法整数，返回 `-ERR invalid expire seconds\r\n`

`TTL key`：

- key 不存在，返回 `:-2\r\n`
- key 存在但没有过期时间，返回 `:-1\r\n`
- key 存在且有过期时间，返回剩余秒数 `:N\r\n`
- key 已过期时，先删除再返回 `:-2\r\n`

## 示例

```text
SET token abc
+OK
EXPIRE token 3
:1
TTL token
:3
GET token
$3
abc
```

等待过期后：

```text
GET token
$-1
TTL token
:-2
```
