#pragma once

#include "lightkv/net/Buffer.h"

#include <cstddef>
#include <string>

namespace lightkv {

class TcpConnection {
public:
    TcpConnection(int fd, std::string peer_addr);

    int fd() const;
    const std::string& peerAddr() const;
    void appendInput(const char* data, std::size_t len);
    bool hasLine() const;
    std::string popLine();
    void markClosed();
    bool closed() const;

private:
    int fd_;
    Buffer input_buffer_;
    std::string peer_addr_;
    bool closed_ = false;
};

}  // namespace lightkv

