#include "lightkv/storage/KVStore.h"

#include <mutex>

namespace lightkv {

Status KVStore::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_[key] = Entry{value};
    return Status::OK();
}

std::optional<std::string> KVStore::get(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    const auto it = data_.find(key);
    if (it == data_.end()) {
        return std::nullopt;
    }

    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return data_.erase(key) > 0;
}

bool KVStore::exists(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.find(key) != data_.end();
}

size_t KVStore::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.size();
}

void KVStore::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.clear();
}

}  // namespace lightkv

