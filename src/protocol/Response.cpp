#include "lightkv/protocol/Response.h"

namespace lightkv {

std::string Response::simpleString(const std::string& value) {
    return "+" + value + "\r\n";
}

std::string Response::error(const std::string& message) {
    return "-ERR " + message + "\r\n";
}

std::string Response::integer(long long value) {
    return ":" + std::to_string(value) + "\r\n";
}

std::string Response::bulkString(const std::string& value) {
    return "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
}

std::string Response::nullBulkString() {
    return "$-1\r\n";
}

}  // namespace lightkv

