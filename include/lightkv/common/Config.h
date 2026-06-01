#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace lightkv {

class Config {
public:
    bool loadFromFile(const std::string& path);

    std::string getString(const std::string& key, const std::string& default_value) const;
    int getInt(const std::string& key, int default_value) const;
    size_t getSizeT(const std::string& key, size_t default_value) const;
    bool getBool(const std::string& key, bool default_value) const;

    void set(const std::string& key, const std::string& value);
    bool contains(const std::string& key) const;

private:
    std::unordered_map<std::string, std::string> values_;
};

std::string trim(const std::string& value);

}  // namespace lightkv

