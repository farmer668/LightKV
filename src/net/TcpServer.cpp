#include "lightkv/net/TcpServer.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <utility>

namespace lightkv {

namespace {

constexpr int kMaxEvents = 64;
constexpr int kBacklog = 128;
constexpr std::size_t kReadBufferSize = 4096;

std::string errnoMessage(const std::string& prefix) {
    return prefix + ": " + std::strerror(errno);
}

}  // namespace

TcpServer::TcpServer(
    std::string host,
    int port,
    size_t max_keys,
    bool wal_enabled,
    std::string wal_path)
    : host_(std::move(host)),
      port_(port),
      wal_enabled_(wal_enabled),
      wal_path_(std::move(wal_path)),
      store_(max_keys),
      wal_(wal_path_),
      executor_(store_, wal_enabled_ ? &wal_ : nullptr, wal_enabled_, wal_path_) {}

TcpServer::~TcpServer() {
    stopExpireWorker();
    wal_.close();

    for (auto& item : connections_) {
        ::close(item.first);
    }
    connections_.clear();

    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
    }
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
    }
}

bool TcpServer::start() {
    ::signal(SIGPIPE, SIG_IGN);

    if (wal_enabled_) {
        replayWalFile(wal_, store_);
        if (!wal_.open()) {
            std::cerr << "failed to open WAL: " << wal_path_ << '\n';
            return false;
        }
    }

    if (!setupListenSocket()) {
        return false;
    }

    epoll_fd_ = ::epoll_create1(0);
    if (epoll_fd_ < 0) {
        std::cerr << errnoMessage("epoll_create1 failed") << '\n';
        return false;
    }

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = listen_fd_;
    if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listen_fd_, &event) < 0) {
        std::cerr << errnoMessage("epoll_ctl add listen fd failed") << '\n';
        return false;
    }

    startExpireWorker();
    return true;
}

void TcpServer::run() {
    std::array<epoll_event, kMaxEvents> events{};

    while (true) {
        const int count = ::epoll_wait(epoll_fd_, events.data(), static_cast<int>(events.size()), -1);
        if (count < 0) {
            if (errno == EINTR) {
                continue;
            }
            std::cerr << errnoMessage("epoll_wait failed") << '\n';
            break;
        }

        for (int i = 0; i < count; ++i) {
            const int fd = events[i].data.fd;
            if (fd == listen_fd_) {
                handleAccept();
                continue;
            }

            if ((events[i].events & (EPOLLERR | EPOLLHUP)) != 0) {
                closeConnection(fd);
                continue;
            }

            if ((events[i].events & EPOLLIN) != 0) {
                handleClientRead(fd);
            }
        }
    }
}

bool TcpServer::setupListenSocket() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        std::cerr << errnoMessage("socket failed") << '\n';
        return false;
    }

    int reuse = 1;
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        std::cerr << errnoMessage("setsockopt SO_REUSEADDR failed") << '\n';
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    if (::inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "invalid bind host: " << host_ << '\n';
        return false;
    }

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << errnoMessage("bind failed") << '\n';
        return false;
    }

    if (::listen(listen_fd_, kBacklog) < 0) {
        std::cerr << errnoMessage("listen failed") << '\n';
        return false;
    }

    if (!setNonBlocking(listen_fd_)) {
        return false;
    }

    return true;
}

void TcpServer::startExpireWorker() {
    running_.store(true);
    expire_thread_ = std::thread([this]() {
        while (running_.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (running_.load()) {
                store_.cleanupExpired();
            }
        }
    });
}

void TcpServer::stopExpireWorker() {
    running_.store(false);
    if (expire_thread_.joinable()) {
        expire_thread_.join();
    }
}

bool TcpServer::setNonBlocking(int fd) {
    const int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        std::cerr << errnoMessage("fcntl F_GETFL failed") << '\n';
        return false;
    }

    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        std::cerr << errnoMessage("fcntl F_SETFL failed") << '\n';
        return false;
    }

    return true;
}

void TcpServer::handleAccept() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        const int client_fd = ::accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            }
            if (errno == EINTR) {
                continue;
            }
            std::cerr << errnoMessage("accept failed") << '\n';
            return;
        }

        if (!setNonBlocking(client_fd)) {
            ::close(client_fd);
            continue;
        }

        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = client_fd;
        if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &event) < 0) {
            std::cerr << errnoMessage("epoll_ctl add client fd failed") << '\n';
            ::close(client_fd);
            continue;
        }

        char ip[INET_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::string peer = std::string(ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
        connections_.emplace(client_fd, TcpConnection(client_fd, peer));
        std::cout << "client connected: " << peer << '\n';
    }
}

void TcpServer::handleClientRead(int client_fd) {
    auto conn_it = connections_.find(client_fd);
    if (conn_it == connections_.end()) {
        closeConnection(client_fd);
        return;
    }

    std::array<char, kReadBufferSize> buffer{};
    while (true) {
        const ssize_t n = ::recv(client_fd, buffer.data(), buffer.size(), 0);
        if (n > 0) {
            conn_it->second.appendInput(buffer.data(), static_cast<std::size_t>(n));

            while (conn_it->second.hasLine()) {
                const auto line = conn_it->second.popLine();
                const auto command = parser_.parseLine(line);
                const auto response = executor_.execute(command);
                if (!sendResponse(client_fd, response)) {
                    closeConnection(client_fd);
                    return;
                }

                if (command.type == CommandType::Quit) {
                    closeConnection(client_fd);
                    return;
                }
            }
            continue;
        }

        if (n == 0) {
            closeConnection(client_fd);
            return;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }

        if (errno == EINTR) {
            continue;
        }

        std::cerr << errnoMessage("recv failed") << '\n';
        closeConnection(client_fd);
        return;
    }
}

bool TcpServer::sendResponse(int client_fd, const std::string& response) {
    std::size_t sent = 0;
    while (sent < response.size()) {
        const ssize_t n = ::send(
            client_fd,
            response.data() + sent,
            response.size() - sent,
            MSG_NOSIGNAL);

        if (n > 0) {
            sent += static_cast<std::size_t>(n);
            continue;
        }

        if (n < 0 && errno == EINTR) {
            continue;
        }

        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            std::cerr << "send would block; closing connection\n";
            return false;
        }

        std::cerr << errnoMessage("send failed") << '\n';
        return false;
    }

    return true;
}

void TcpServer::closeConnection(int client_fd) {
    auto conn_it = connections_.find(client_fd);
    if (conn_it != connections_.end()) {
        conn_it->second.markClosed();
        std::cout << "client disconnected: " << conn_it->second.peerAddr() << '\n';
        connections_.erase(conn_it);
    }

    if (epoll_fd_ >= 0) {
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr);
    }
    ::close(client_fd);
}

}  // namespace lightkv
