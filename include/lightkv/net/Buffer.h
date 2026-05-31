#pragma once

#include <cstddef>
#include <string>

namespace lightkv {

class Buffer {
public:
    void append(const char* data, std::size_t len);
    bool hasLine() const;
    std::string popLine();
    std::size_t size() const;
    void clear();

private:
    std::string data_;
};

}  // namespace lightkv

