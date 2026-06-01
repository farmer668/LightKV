#include "lightkv/client/LightKVClient.h"

#include <string>
#include <utility>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#endif

namespace lightkv {

namespace {

std::string ensureLineEnding(std::string command) {
    if (command.size() >= 2 && command.substr(command.size() - 2) == "\r\n") {
        return command;
    }
    if (!command.empty() && command.back() == '\n') {
        command.pop_back();
    }
    return command + "\r\n";
}

#if !defined(_WIN32)
bool recvByte(int fd, char& ch) {
    while (true) {
        const ssize_t n = ::recv(fd, &ch, 1, 0);
        if (n == 1) {
            return true;
        }
        if (n < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
}

bool readLine(int fd, std::string& line) {
    line.clear();
    char ch = '\0';
    while (recvByte(fd, ch)) {
        line.push_back(ch);
        if (line.size() >= 2 && line.substr(line.size() - 2) == "\r\n") {
            return true;
        }
    }
    return false;
}

bool readExact(int fd, size_t bytes, std::string& output) {
    output.clear();
    output.reserve(bytes);
    while (output.size() < bytes) {
        char buffer[4096];
        const auto remaining = bytes - output.size();
        const auto want = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        const ssize_t n = ::recv(fd, buffer, want, 0);
        if (n > 0) {
            output.append(buffer, static_cast<size_t>(n));
            continue;
        }
        if (n < 0 && errno == EINTR) {
            continue;
        }
        return false;
    }
    return true;
}

std::string readResponse(int fd) {
    std::string first_line;
    if (!readLine(fd, first_line)) {
        return "-ERR connection failed while reading response\r\n";
    }

    if (first_line.empty() || first_line[0] != '$') {
        return first_line;
    }

    const auto size_text = first_line.substr(1, first_line.size() - 3);
    long long bulk_size = -1;
    try {
        bulk_size = std::stoll(size_text);
    } catch (...) {
        return "-ERR invalid bulk string response\r\n";
    }

    if (bulk_size < 0) {
        return first_line;
    }

    std::string body;
    if (!readExact(fd, static_cast<size_t>(bulk_size) + 2, body)) {
        return "-ERR connection failed while reading bulk string\r\n";
    }
    return first_line + body;
}
#endif

}  // namespace

LightKVClient::LightKVClient(std::string host, int port)
    : host_(std::move(host)), port_(port) {}

std::string LightKVClient::sendCommand(const std::string& command) {
#if defined(_WIN32)
    (void)command;
    return "-ERR LightKVClient TCP is only available on Linux/Unix\r\n";
#else
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return "-ERR connection failed: socket\r\n";
    }

    timeval timeout{};
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port_));
    if (::inet_pton(AF_INET, host_.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return "-ERR connection failed: invalid host\r\n";
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        const std::string message = "-ERR connection failed: " + std::string(std::strerror(errno)) + "\r\n";
        ::close(fd);
        return message;
    }

    const auto request = ensureLineEnding(command);
    size_t sent = 0;
    while (sent < request.size()) {
        const ssize_t n = ::send(fd, request.data() + sent, request.size() - sent, 0);
        if (n > 0) {
            sent += static_cast<size_t>(n);
            continue;
        }
        if (n < 0 && errno == EINTR) {
            continue;
        }
        const std::string message = "-ERR connection failed: send\r\n";
        ::close(fd);
        return message;
    }

    const auto response = readResponse(fd);
    ::close(fd);
    return response;
#endif
}

std::string LightKVClient::set(const std::string& key, const std::string& value) {
    return sendCommand("SET " + key + " " + value);
}

std::string LightKVClient::get(const std::string& key) {
    return sendCommand("GET " + key);
}

std::string LightKVClient::del(const std::string& key) {
    return sendCommand("DEL " + key);
}

std::string LightKVClient::expire(const std::string& key, int seconds) {
    return sendCommand("EXPIRE " + key + " " + std::to_string(seconds));
}

std::string LightKVClient::ttl(const std::string& key) {
    return sendCommand("TTL " + key);
}

std::string LightKVClient::info() {
    return sendCommand("INFO");
}

}  // namespace lightkv
