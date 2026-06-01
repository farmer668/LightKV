#include "lightkv/protocol/CommandExecutor.h"

#include "lightkv/protocol/Response.h"

#include <cstddef>
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

}  // namespace

CommandExecutor::CommandExecutor(
    KVStore& store,
    Wal* wal,
    bool wal_enabled,
    std::string wal_path)
    : store_(store),
      wal_(wal),
      wal_enabled_(wal_enabled),
      wal_path_(std::move(wal_path)) {}

std::string CommandExecutor::execute(const Command& command) {
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
                return Response::nullBulkString();
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
                return Response::bulkString(info.str());
            }
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
