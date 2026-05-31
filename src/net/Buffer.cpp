#include "lightkv/net/Buffer.h"

namespace lightkv {

void Buffer::append(const char* data, std::size_t len) {
    data_.append(data, len);
}

bool Buffer::hasLine() const {
    return data_.find('\n') != std::string::npos;
}

std::string Buffer::popLine() {
    const auto pos = data_.find('\n');
    if (pos == std::string::npos) {
        return "";
    }

    std::string line = data_.substr(0, pos);
    data_.erase(0, pos + 1);

    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    return line;
}

std::size_t Buffer::size() const {
    return data_.size();
}

void Buffer::clear() {
    data_.clear();
}

}  // namespace lightkv

