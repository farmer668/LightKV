#include "lightkv/net/TcpConnection.h"

#include <utility>

namespace lightkv {

TcpConnection::TcpConnection(int fd, std::string peer_addr)
    : fd_(fd), peer_addr_(std::move(peer_addr)) {}

int TcpConnection::fd() const {
    return fd_;
}

const std::string& TcpConnection::peerAddr() const {
    return peer_addr_;
}

void TcpConnection::appendInput(const char* data, std::size_t len) {
    input_buffer_.append(data, len);
}

bool TcpConnection::hasLine() const {
    return input_buffer_.hasLine();
}

std::string TcpConnection::popLine() {
    return input_buffer_.popLine();
}

void TcpConnection::markClosed() {
    closed_ = true;
}

bool TcpConnection::closed() const {
    return closed_;
}

}  // namespace lightkv

