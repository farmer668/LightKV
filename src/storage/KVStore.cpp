#include "lightkv/storage/KVStore.h"

#include <chrono>
#include <mutex>

namespace lightkv {

Status KVStore::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    Entry entry;
    entry.value = value;
    data_[key] = entry;
    return Status::OK();
}

std::optional<std::string> KVStore::get(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return std::nullopt;
    }

    const auto now = std::chrono::steady_clock::now();
    if (eraseIfExpiredLocked(it, now)) {
        return std::nullopt;
    }

    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return data_.erase(key) > 0;
}

bool KVStore::exists(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }

    const auto now = std::chrono::steady_clock::now();
    if (eraseIfExpiredLocked(it, now)) {
        return false;
    }

    return true;
}

size_t KVStore::size() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    for (auto it = data_.begin(); it != data_.end();) {
        if (isExpired(it->second, now)) {
            it = data_.erase(it);
        } else {
            ++it;
        }
    }
    return data_.size();
}

void KVStore::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.clear();
}

bool KVStore::expire(const std::string& key, int seconds) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return false;
    }

    if (eraseIfExpiredLocked(it, now)) {
        return false;
    }

    if (seconds <= 0) {
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
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return -2;
    }

    if (eraseIfExpiredLocked(it, now)) {
        return -2;
    }

    if (!it->second.has_ttl) {
        return -1;
    }

    const auto remaining = it->second.expire_at - now;
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(remaining).count();
    if (milliseconds <= 0) {
        data_.erase(it);
        return -2;
    }

    return (milliseconds + 999) / 1000;
}

size_t KVStore::cleanupExpired(size_t max_scan) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto now = std::chrono::steady_clock::now();
    size_t scanned = 0;
    size_t removed = 0;

    for (auto it = data_.begin(); it != data_.end() && scanned < max_scan;) {
        ++scanned;
        if (isExpired(it->second, now)) {
            it = data_.erase(it);
            ++removed;
        } else {
            ++it;
        }
    }

    return removed;
}

bool KVStore::isExpired(const Entry& entry, std::chrono::steady_clock::time_point now) const {
    return entry.has_ttl && now >= entry.expire_at;
}

bool KVStore::eraseIfExpiredLocked(
    std::unordered_map<std::string, Entry>::iterator it,
    std::chrono::steady_clock::time_point now) {
    if (!isExpired(it->second, now)) {
        return false;
    }

    data_.erase(it);
    return true;
}

}  // namespace lightkv
