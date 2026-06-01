#include "lightkv/protocol/Parser.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace lightkv {

namespace {

std::string toUpper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

Command invalidCommand(const std::string& raw, std::string error) {
    Command command;
    command.type = CommandType::Invalid;
    command.raw = raw;
    command.error = std::move(error);
    return command;
}

bool expectArgCount(Command& command, size_t expected) {
    if (command.args.size() == expected) {
        return true;
    }

    command.type = CommandType::Invalid;
    command.error = "wrong number of arguments";
    return false;
}

}  // namespace

Command Parser::parseLine(const std::string& line) const {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        return invalidCommand(line, "empty command");
    }

    static const std::unordered_map<std::string, CommandType> command_types = {
        {"PING", CommandType::Ping},
        {"SET", CommandType::Set},
        {"GET", CommandType::Get},
        {"DEL", CommandType::Del},
        {"EXISTS", CommandType::Exists},
        {"EXPIRE", CommandType::Expire},
        {"INFO", CommandType::Info},
        {"SIZE", CommandType::Size},
        {"TTL", CommandType::Ttl},
        {"CLEAR", CommandType::Clear},
        {"QUIT", CommandType::Quit},
    };

    const auto name = toUpper(tokens.front());
    const auto type_it = command_types.find(name);
    if (type_it == command_types.end()) {
        return invalidCommand(line, "unknown command");
    }

    Command command;
    command.type = type_it->second;
    command.raw = line;
    command.args.assign(tokens.begin() + 1, tokens.end());

    switch (command.type) {
        case CommandType::Ping:
        case CommandType::Info:
        case CommandType::Size:
        case CommandType::Clear:
        case CommandType::Quit:
            expectArgCount(command, 0);
            break;
        case CommandType::Get:
        case CommandType::Del:
        case CommandType::Exists:
        case CommandType::Ttl:
            expectArgCount(command, 1);
            break;
        case CommandType::Set:
        case CommandType::Expire:
            expectArgCount(command, 2);
            break;
        case CommandType::Invalid:
            break;
    }

    return command;
}

}  // namespace lightkv
