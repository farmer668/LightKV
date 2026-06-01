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

## INFO 字段

- `keys`：当前 key 数量。
- `max_keys`：KVStore 容量上限。
- `evicted_keys`：LRU 淘汰次数。
- `expired_keys`：TTL 过期删除次数。
- `wal_enabled`：WAL 是否启用。
- `wal_path`：WAL 文件路径。
- `wal_records`：当前进程写入 WAL 的记录数。
- `total_commands`：执行过的命令数量。
- `ping_commands`、`get_commands`、`set_commands`、`del_commands`、`exists_commands`、`expire_commands`、`ttl_commands`、`info_commands`：按命令类型统计。
- `hits`：GET 命中次数。
- `misses`：GET 未命中次数。
- `connected_clients`：当前连接数。
- `total_connections`：累计连接数。
- `current_connections`：当前连接数。

## WAL 行为

- `SET key value` 成功后写入 WAL。
- `DEL key` 成功删除 key 后写入 WAL。
- `EXPIRE key seconds` 成功后写入 WAL。
- `GET`、`EXISTS`、`TTL`、`SIZE`、`INFO`、`CLEAR` 不写 WAL。
- `CLEAR` 当前不参与 WAL 持久化。
