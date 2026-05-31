#pragma once

#include <string>
#include <vector>

namespace lightkv {

enum class CommandType {
    Ping,
    Set,
    Get,
    Del,
    Exists,
    Expire,
    Size,
    Ttl,
    Clear,
    Quit,
    Invalid
};

struct Command {
    CommandType type = CommandType::Invalid;
    std::vector<std::string> args;
    std::string raw;
    std::string error;
};

}  // namespace lightkv
