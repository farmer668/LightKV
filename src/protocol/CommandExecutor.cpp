#include "lightkv/protocol/CommandExecutor.h"

#include "lightkv/protocol/Response.h"
#include "lightkv/replication/MasterReplication.h"
#include "lightkv/replication/ReplicationRole.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <utility>

namespace lightkv {

namespace {

bool hasArgCount(const Command& command, std::size_t expected) {
    return command.args.size() == expected;
}

bool parseSeconds(const std::string& text, int& seconds) {
    try {
        std::size_t parsed = 0;
        const long long value = std::stoll(text, &parsed, 10);
        if (parsed != text.size() ||
            value < std::numeric_limits<int>::min() ||
            value > std::numeric_limits<int>::max()) {
            return false;
        }
        seconds = static_cast<int>(value);
        return true;
    } catch (...) {
        return false;
    }
}

void recordCommand(Metrics* metrics, CommandType type) {
    if (metrics == nullptr) {
        return;
    }

    metrics->incTotalCommands();
    switch (type) {
        case CommandType::Ping:
            metrics->incPingCommands();
            break;
        case CommandType::Set:
            metrics->incSetCommands();
            break;
        case CommandType::Get:
            metrics->incGetCommands();
            break;
        case CommandType::Del:
            metrics->incDelCommands();
            break;
        case CommandType::Exists:
            metrics->incExistsCommands();
            break;
        case CommandType::Expire:
            metrics->incExpireCommands();
            break;
        case CommandType::Ttl:
            metrics->incTtlCommands();
            break;
        case CommandType::Info:
            metrics->incInfoCommands();
            break;
        case CommandType::Sync:
        case CommandType::ReplConf:
        case CommandType::Size:
        case CommandType::Clear:
        case CommandType::Quit:
        case CommandType::Invalid:
            break;
    }
}

bool isWriteCommand(CommandType type) {
    return type == CommandType::Set ||
           type == CommandType::Del ||
           type == CommandType::Expire ||
           type == CommandType::Clear;
}

bool parseOffset(const std::string& text, uint64_t& offset) {
    try {
        std::size_t parsed = 0;
        const auto value = std::stoull(text, &parsed, 10);
        if (parsed != text.size()) {
            return false;
        }
        offset = static_cast<uint64_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

CommandExecutor::CommandExecutor(
    KVStore& store,
    Wal* wal,
    bool wal_enabled,
    std::string wal_path,
    Metrics* metrics,
    ReplicationState* replication_state)
    : store_(store),
      wal_(wal),
      wal_enabled_(wal_enabled),
      wal_path_(std::move(wal_path)),
      metrics_(metrics),
      replication_state_(replication_state) {}

std::string CommandExecutor::execute(const Command& command) {
    recordCommand(metrics_, command.type);

    if (replication_state_ != nullptr &&
        replication_state_->role() == ReplicationRole::Slave &&
        isWriteCommand(command.type)) {
        return Response::error("slave is read-only");
    }

    switch (command.type) {
        case CommandType::Ping:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            return Response::simpleString("PONG");
        case CommandType::Set: {
            if (!hasArgCount(command, 2)) {
                return Response::error("wrong number of arguments");
            }
            const auto status = store_.set(command.args[0], command.args[1]);
            if (!status.ok()) {
                return Response::error(status.toString());
            }
            if (wal_enabled_ && wal_ != nullptr) {
                wal_->appendSet(command.args[0], command.args[1]);
            }
            return Response::simpleString("OK");
        }
        case CommandType::Get: {
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            const auto value = store_.get(command.args[0]);
            if (!value.has_value()) {
                if (metrics_ != nullptr) {
                    metrics_->incMisses();
                }
                return Response::nullBulkString();
            }
            if (metrics_ != nullptr) {
                metrics_->incHits();
            }
            return Response::bulkString(value.value());
        }
        case CommandType::Del:
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            if (store_.del(command.args[0])) {
                if (wal_enabled_ && wal_ != nullptr) {
                    wal_->appendDel(command.args[0]);
                }
                return Response::integer(1);
            }
            return Response::integer(0);
        case CommandType::Exists:
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            return Response::integer(store_.exists(command.args[0]) ? 1 : 0);
        case CommandType::Expire: {
            if (!hasArgCount(command, 2)) {
                return Response::error("wrong number of arguments");
            }
            int seconds = 0;
            if (!parseSeconds(command.args[1], seconds)) {
                return Response::error("invalid expire seconds");
            }
            if (store_.expire(command.args[0], seconds)) {
                if (wal_enabled_ && wal_ != nullptr) {
                    if (seconds <= 0) {
                        wal_->appendDel(command.args[0]);
                    } else {
                        wal_->appendExpire(command.args[0], seconds);
                    }
                }
                return Response::integer(1);
            }
            return Response::integer(0);
        }
        case CommandType::Info:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            {
                std::ostringstream info;
                info << store_.info() << '\n';
                info << "wal_enabled:" << (wal_enabled_ ? "true" : "false") << '\n';
                info << "wal_path:" << wal_path_ << '\n';
                info << "wal_records:" << ((wal_enabled_ && wal_ != nullptr) ? wal_->recordsWritten() : 0);
                info << '\n' << "wal_last_offset:" << ((wal_enabled_ && wal_ != nullptr) ? wal_->lastOffset() : 0);
                if (replication_state_ != nullptr) {
                    const auto master_offset = (wal_ != nullptr) ? wal_->lastOffset() : 0;
                    info << '\n' << replication_state_->info(master_offset);
                }
                if (metrics_ != nullptr) {
                    info << '\n' << metrics_->toString();
                }
                return Response::bulkString(info.str());
            }
        case CommandType::Sync: {
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            if (replication_state_ != nullptr &&
                replication_state_->role() == ReplicationRole::Slave) {
                return Response::error("only master can sync");
            }
            if (wal_ == nullptr) {
                return Response::bulkString("");
            }
            uint64_t offset = 0;
            if (!parseOffset(command.args[0], offset)) {
                return Response::error("invalid sync offset");
            }
            MasterReplication replication(*wal_);
            return Response::bulkString(replication.syncPayloadAfter(offset));
        }
        case CommandType::ReplConf:
            return Response::simpleString("OK");
        case CommandType::Size:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            return Response::integer(static_cast<long long>(store_.size()));
        case CommandType::Ttl:
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            return Response::integer(store_.ttl(command.args[0]));
        case CommandType::Clear:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            store_.clear();
            return Response::simpleString("OK");
        case CommandType::Quit:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            return Response::simpleString("BYE");
        case CommandType::Invalid:
            return Response::error(command.error.empty() ? "invalid command" : command.error);
    }

    return Response::error("invalid command");
}

}  // namespace lightkv
