#include "lightkv/storage/LRUCache.h"

namespace lightkv {

void LRUCache::touch(const std::string& key) {
    const auto it = positions_.find(key);
    if (it != positions_.end()) {
        keys_.erase(it->second);
    }

    keys_.push_front(key);
    positions_[key] = keys_.begin();
}

void LRUCache::remove(const std::string& key) {
    const auto it = positions_.find(key);
    if (it == positions_.end()) {
        return;
    }

    keys_.erase(it->second);
    positions_.erase(it);
}

bool LRUCache::contains(const std::string& key) const {
    return positions_.find(key) != positions_.end();
}

std::optional<std::string> LRUCache::evictCandidate() const {
    if (keys_.empty()) {
        return std::nullopt;
    }

    return keys_.back();
}

void LRUCache::clear() {
    keys_.clear();
    positions_.clear();
}

size_t LRUCache::size() const {
    return keys_.size();
}

}  // namespace lightkv

