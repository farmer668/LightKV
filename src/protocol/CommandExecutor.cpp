#include "lightkv/protocol/CommandExecutor.h"

#include "lightkv/protocol/Response.h"

#include <cstddef>

namespace lightkv {

namespace {

bool hasArgCount(const Command& command, std::size_t expected) {
    return command.args.size() == expected;
}

}  // namespace

CommandExecutor::CommandExecutor(KVStore& store) : store_(store) {}

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
            return Response::integer(store_.del(command.args[0]) ? 1 : 0);
        case CommandType::Exists:
            if (!hasArgCount(command, 1)) {
                return Response::error("wrong number of arguments");
            }
            return Response::integer(store_.exists(command.args[0]) ? 1 : 0);
        case CommandType::Size:
            if (!hasArgCount(command, 0)) {
                return Response::error("wrong number of arguments");
            }
            return Response::integer(static_cast<long long>(store_.size()));
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
