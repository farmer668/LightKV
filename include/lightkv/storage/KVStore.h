#pragma once

#include "lightkv/common/Status.h"
#include "lightkv/storage/Entry.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace lightkv {

class KVStore {
public:
    Status set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    size_t size();
    void clear();
    bool expire(const std::string& key, int seconds);
    long long ttl(const std::string& key);
    size_t cleanupExpired(size_t max_scan = 100);

private:
    bool isExpired(const Entry& entry, std::chrono::steady_clock::time_point now) const;
    bool eraseIfExpiredLocked(
        std::unordered_map<std::string, Entry>::iterator it,
        std::chrono::steady_clock::time_point now);

    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Entry> data_;
};

}  // namespace lightkv
