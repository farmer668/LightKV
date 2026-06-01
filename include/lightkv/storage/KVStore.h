#pragma once

#include "lightkv/common/Status.h"
#include "lightkv/storage/Entry.h"
#include "lightkv/storage/LRUCache.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace lightkv {

class KVStore {
public:
    explicit KVStore(size_t max_keys = 10000);

    Status set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    size_t size();
    void clear();
    bool expire(const std::string& key, int seconds);
    long long ttl(const std::string& key);
    size_t cleanupExpired(size_t max_scan = 100);
    size_t maxKeys() const;
    size_t evictedKeys() const;
    size_t expiredKeys() const;
    std::string info();

private:
    using EntryMap = std::unordered_map<std::string, Entry>;

    bool isExpired(const Entry& entry, std::chrono::steady_clock::time_point now) const;
    bool eraseIfExpiredLocked(EntryMap::iterator it, std::chrono::steady_clock::time_point now);
    EntryMap::iterator findLiveEntryLocked(
        const std::string& key,
        std::chrono::steady_clock::time_point now);
    void evictIfNeededLocked();
    size_t cleanupExpiredLocked(size_t max_scan);

    mutable std::shared_mutex mutex_;
    size_t max_keys_;
    EntryMap data_;
    LRUCache lru_;
    size_t evicted_keys_ = 0;
    size_t expired_keys_ = 0;
};

}  // namespace lightkv
