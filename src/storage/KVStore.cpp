#include "lightkv/storage/KVStore.h"

#include <chrono>
#include <limits>
#include <mutex>
#include <sstream>

namespace lightkv {

KVStore::KVStore(size_t max_keys) : max_keys_(max_keys) {}

Status KVStore::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const bool is_new_key = data_.find(key) == data_.end();

    Entry entry;
    entry.value = value;
    data_[key] = entry;
    lru_.touch(key);

    if (is_new_key) {
        evictIfNeededLocked();
    }

    return Status::OK();
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    const auto it = findLiveEntryLocked(key, now);
    if (it == data_.end()) {
        return std::nullopt;
    }

    lru_.touch(key);
    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto removed = data_.erase(key) > 0;
    if (removed) {
        lru_.remove(key);
    }
    return removed;
}

bool KVStore::exists(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    return findLiveEntryLocked(key, now) != data_.end();
}

size_t KVStore::size() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cleanupExpiredLocked(std::numeric_limits<size_t>::max());
    return data_.size();
}

void KVStore::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.clear();
    lru_.clear();
}

bool KVStore::expire(const std::string& key, int seconds) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    const auto it = findLiveEntryLocked(key, now);
    if (it == data_.end()) {
        return false;
    }

    if (seconds <= 0) {
        lru_.remove(key);
        data_.erase(it);
        return true;
    }

    it->second.has_ttl = true;
    it->second.expire_at = now + std::chrono::seconds(seconds);
    return true;
}

long long KVStore::ttl(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    const auto it = findLiveEntryLocked(key, now);
    if (it == data_.end()) {
        return -2;
    }

    if (!it->second.has_ttl) {
        return -1;
    }

    const auto remaining = it->second.expire_at - now;
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(remaining).count();
    return (milliseconds + 999) / 1000;
}

size_t KVStore::cleanupExpired(size_t max_scan) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return cleanupExpiredLocked(max_scan);
}

size_t KVStore::maxKeys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return max_keys_;
}

size_t KVStore::evictedKeys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return evicted_keys_;
}

size_t KVStore::expiredKeys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return expired_keys_;
}

std::string KVStore::info() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cleanupExpiredLocked(std::numeric_limits<size_t>::max());

    std::ostringstream out;
    out << "keys:" << data_.size() << '\n';
    out << "max_keys:" << max_keys_ << '\n';
    out << "evicted_keys:" << evicted_keys_ << '\n';
    out << "expired_keys:" << expired_keys_;
    return out.str();
}

bool KVStore::isExpired(const Entry& entry, std::chrono::steady_clock::time_point now) const {
    return entry.has_ttl && now >= entry.expire_at;
}

bool KVStore::eraseIfExpiredLocked(
    EntryMap::iterator it,
    std::chrono::steady_clock::time_point now) {
    if (!isExpired(it->second, now)) {
        return false;
    }

    lru_.remove(it->first);
    data_.erase(it);
    ++expired_keys_;
    return true;
}

KVStore::EntryMap::iterator KVStore::findLiveEntryLocked(
    const std::string& key,
    std::chrono::steady_clock::time_point now) {
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return data_.end();
    }

    if (eraseIfExpiredLocked(it, now)) {
        return data_.end();
    }

    return it;
}

void KVStore::evictIfNeededLocked() {
    if (max_keys_ == 0) {
        return;
    }

    while (data_.size() > max_keys_) {
        const auto candidate = lru_.evictCandidate();
        if (!candidate.has_value()) {
            return;
        }

        lru_.remove(candidate.value());
        if (data_.erase(candidate.value()) > 0) {
            ++evicted_keys_;
        }
    }
}

size_t KVStore::cleanupExpiredLocked(size_t max_scan) {
    const auto now = std::chrono::steady_clock::now();
    size_t scanned = 0;
    size_t removed = 0;

    for (auto it = data_.begin(); it != data_.end() && scanned < max_scan;) {
        ++scanned;
        if (isExpired(it->second, now)) {
            lru_.remove(it->first);
            it = data_.erase(it);
            ++removed;
            ++expired_keys_;
        } else {
            ++it;
        }
    }

    return removed;
}

}  // namespace lightkv
