# Debug Log

## Stage 4 bugfix: GET returned null before TTL expired

Reproduction:

```text
SET token abc
EXPIRE token 10
TTL token
GET token
```

Wrong behavior:

- `TTL token` returned a positive value, such as `:6`.
- `GET token` returned `$-1`.

Root cause:

- TTL live-entry access was not centralized.
- `get`, `exists`, `ttl`, and `expire` needed to share the same expiration rule.

Fix:

- Added shared live-entry lookup helpers.
- Kept the exact expiration condition as `entry.has_ttl && now >= entry.expire_at`.
- Added regression coverage to ensure positive TTL means GET returns the value.

## Stage 8

- No major issues.
- Windows validation covered WAL offset, WAL replay, parser, replication state, and CommandExecutor behavior.
- Full Linux TCP master/slave `nc` verification is left to Ubuntu.

## Stage 9

- No major issues.
- Windows validation covered consistent hashing, ClusterClient routing, and cluster CLI parsing.
- Full Linux TCP client and multi-node cluster CLI verification is left to Ubuntu.

## Stage 10

- No major issues.
- Windows validation covered benchmark option parsing and benchmark smoke execution.
- Real benchmark results are intentionally not fabricated and should be filled after Ubuntu pressure testing.
