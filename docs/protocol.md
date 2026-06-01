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

## WAL 行为

- `SET key value` 成功后写入 WAL。
- `DEL key` 成功删除 key 后写入 WAL。
- `EXPIRE key seconds` 成功后写入 WAL。
- `GET`、`EXISTS`、`TTL`、`SIZE`、`INFO`、`CLEAR` 不写 WAL。
- `CLEAR` 当前不参与 WAL 持久化。

## INFO

`INFO` 返回 bulk string，内容字段包括：

```text
keys:3
max_keys:10000
evicted_keys:0
expired_keys:0
wal_enabled:true
wal_path:data/lightkv.wal
wal_records:3
```

## WAL 限制

- value 暂不支持空格。
- `EXPIRE` replay 使用相对时间。
- 暂未实现 WAL rewrite / compaction。
