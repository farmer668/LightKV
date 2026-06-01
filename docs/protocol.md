# 协议设计

LightKV 当前支持本地 CLI 和 Linux TCP Server 的文本命令协议。当前不是完整 Redis 协议，只使用简化 RESP 风格响应。

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
| INFO | 0 | `INFO` |
| SIZE | 0 | `SIZE` |
| CLEAR | 0 | `CLEAR` |
| QUIT | 0 | `QUIT` |

## 响应格式

- Simple string：`+OK\r\n`
- Error：`-ERR message\r\n`
- Integer：`:1\r\n`
- Bulk string：`$5\r\nyifei\r\n`
- Null bulk string：`$-1\r\n`

## INFO

`INFO` 返回 bulk string，内容字段包括：

```text
keys:3
max_keys:10000
evicted_keys:0
expired_keys:0
```

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
