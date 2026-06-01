# Debug Log

## Stage 4 bugfix: GET returned null before TTL expired

- Reproduction: `PING`, `SET token abc`, `EXPIRE token 10`, `TTL token`, `GET token`.
- Wrong behavior: `TTL token` returned a positive value, but `GET token` returned `$-1`.
- Root cause: TTL live-entry access was not centralized, which made it easy to delete keys that had TTL but had not expired.
- Fix: shared `findLiveEntryLocked()` / `eraseIfExpiredLocked()` logic and only expire when `has_ttl == true && now >= expire_at`.
- Verification: added TTL and CommandExecutor regression tests.

## Stage 4 bugfix: GET returned null while TTL was still positive

- Reproduction: `SET token abc`, `EXPIRE token 10`, `TTL token`, `GET token`.
- Wrong behavior: `TTL token` returned a positive value such as `:6`, but `GET token` returned `$-1`.
- Root cause: `get`, `exists`, `ttl`, and `expire` did not share one live-entry path.
- Fix: route those methods through the same live-entry helper and keep the exact expiration condition as `has_ttl && now >= expire_at`.
- Verification: `test_ttl_regression` and CommandExecutor tests ensure positive TTL implies GET returns the value.

## Stage 5

- No major issues.

## Stage 6

- No major issues.

## Stage 7

- No major issues.

## Stage 8

- No major issues.
- Windows g++ validation covers platform-independent WAL offset, replayer, parser, replication state, and CommandExecutor behavior.
- Full Linux TCP master/slave `nc` validation is left to the Ubuntu VM.

## Stage 9

- No major issues.
- Windows g++ validation covers consistent hashing, ClusterClient routing, and cluster CLI parsing.
- Full Linux TCP client and multi-node cluster CLI verification is left to the Ubuntu VM.
