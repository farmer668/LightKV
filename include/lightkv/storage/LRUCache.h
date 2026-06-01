#pragma once

#include <cstddef>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>

namespace lightkv {

class LRUCache {
public:
    void touch(const std::string& key);
    void remove(const std::string& key);
    bool contains(const std::string& key) const;
    std::optional<std::string> evictCandidate() const;
    void clear();
    size_t size() const;

private:
    std::list<std::string> keys_;
    std::unordered_map<std::string, std::list<std::string>::iterator> positions_;
};

}  // namespace lightkv
