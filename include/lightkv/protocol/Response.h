#pragma once

#include <string>

namespace lightkv {

class Response {
public:
    static std::string simpleString(const std::string& value);
    static std::string error(const std::string& message);
    static std::string integer(long long value);
    static std::string bulkString(const std::string& value);
    static std::string nullBulkString();
};

}  // namespace lightkv

