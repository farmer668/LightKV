#pragma once

#include <chrono>
#include <string>

namespace lightkv {

struct Entry {
    std::string value;
    bool has_ttl = false;
    std::chrono::steady_clock::time_point expire_at;
};

}  // namespace lightkv

