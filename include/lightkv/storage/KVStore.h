#pragma once

#include "lightkv/common/Status.h"
#include "lightkv/storage/Entry.h"

#include <cstddef>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace lightkv {

class KVStore {
public:
    Status set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool del(const std::string& key);
    bool exists(const std::string& key) const;
    size_t size() const;
    void clear();

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Entry> data_;
};

}  // namespace lightkv
