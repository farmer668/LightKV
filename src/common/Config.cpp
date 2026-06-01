#include "lightkv/common/Config.h"

#include "lightkv/common/Logger.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>

namespace lightkv {

namespace {

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

}  // namespace

bool Config::loadFromFile(const std::string& path) {
    std::ifstream input(path);
    if (!input.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line.front() == '#') {
            continue;
        }

        const auto pos = line.find('=');
        if (pos == std::string::npos) {
            Logger::instance().warn("skip invalid config line: " + line);
            continue;
        }

        const auto key = trim(line.substr(0, pos));
        const auto value = trim(line.substr(pos + 1));
        if (!key.empty()) {
            values_[key] = value;
        }
    }

    return true;
}

std::string Config::getString(const std::string& key, const std::string& default_value) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return default_value;
    }
    return it->second;
}

int Config::getInt(const std::string& key, int default_value) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return default_value;
    }

    try {
        std::size_t parsed = 0;
        const int value = std::stoi(it->second, &parsed, 10);
        if (parsed == it->second.size()) {
            return value;
        }
    } catch (...) {
    }

    Logger::instance().warn("invalid int config for " + key + ", using default");
    return default_value;
}

size_t Config::getSizeT(const std::string& key, size_t default_value) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return default_value;
    }

    try {
        std::size_t parsed = 0;
        const unsigned long long value = std::stoull(it->second, &parsed, 10);
        if (parsed == it->second.size()) {
            return static_cast<size_t>(value);
        }
    } catch (...) {
    }

    Logger::instance().warn("invalid size_t config for " + key + ", using default");
    return default_value;
}

bool Config::getBool(const std::string& key, bool default_value) const {
    const auto it = values_.find(key);
    if (it == values_.end()) {
        return default_value;
    }

    const auto value = toLower(trim(it->second));
    if (value == "true" || value == "1" || value == "yes") {
        return true;
    }
    if (value == "false" || value == "0" || value == "no") {
        return false;
    }

    Logger::instance().warn("invalid bool config for " + key + ", using default");
    return default_value;
}

void Config::set(const std::string& key, const std::string& value) {
    values_[key] = value;
}

bool Config::contains(const std::string& key) const {
    return values_.find(key) != values_.end();
}

std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(begin, end - begin + 1);
}

}  // namespace lightkv
