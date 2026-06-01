#include "lightkv/replication/SlaveReplication.h"

#include "lightkv/common/Logger.h"

#include <algorithm>
#include <cstdint>
#include <sstream>
#include <vector>

#if !defined(_WIN32)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#endif

namespace lightkv {

SlaveReplication::SlaveReplication(
    ReplicationState& state,
    KVStore& store,
    std::chrono::milliseconds interval)
    : state_(state), store_(store), interval_(interval) {}

SlaveReplication::~SlaveReplication() {
    stop();
}

void SlaveReplication::start() {
    if (running_.exchange(true)) {
        return;
    }

    thread_ = std::thread([this]() {
        while (running_.load()) {
            syncOnce();
            std::this_thread::sleep_for(interval_);
        }
    });
}

void SlaveReplication::stop() {
    running_.store(false);
    if (thread_.joinable()) {
        thread_.join();
    }
}

bool SlaveReplication::syncOnce() {
    const auto current_offset = state_.replicationOffset();
    std::string payload;
    if (!requestSyncPayload(current_offset, payload)) {
        state_.recordSync(current_offset, 0, "ERROR", false);
        return false;
    }

    uint64_t last_offset = current_offset;
    const auto applied = applyPayload(payload, last_offset);
    state_.recordSync(last_offset, applied, "OK", true);
    return true;
}

bool SlaveReplication::requestSyncPayload(uint64_t offset, std::string& payload) {
#if defined(_WIN32)
    (void)offset;
    (void)payload;
    Logger::instance().warn("Slave replication network sync is only available on Linux/Unix");
    return false;
#else
    const int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(state_.masterPort()));
    if (::inet_pton(AF_INET, state_.masterHost().c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return false;
    }

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return false;
    }

    const std::string request = "SYNC " + std::to_string(offset) + "\r\n";
    if (::send(fd, request.data(), request.size(), 0) < 0) {
        ::close(fd);
        return false;
    }

    std::string response;
    char buffer[4096];
    while (true) {
        const ssize_t n = ::recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            break;
        }
        response.append(buffer, static_cast<size_t>(n));

        if (!response.empty() && response[0] == '$') {
            const auto header_end = response.find("\r\n");
            if (header_end != std::string::npos) {
                try {
                    const auto length =
                        static_cast<size_t>(std::stoul(response.substr(1, header_end - 1)));
                    const auto total = header_end + 2 + length + 2;
                    if (response.size() >= total) {
                        break;
                    }
                } catch (...) {
                    ::close(fd);
                    return false;
                }
            }
        }
    }
    ::close(fd);

    if (response.empty() || response[0] != '$') {
        return false;
    }

    const auto header_end = response.find("\r\n");
    if (header_end == std::string::npos) {
        return false;
    }

    try {
        const auto length = static_cast<size_t>(std::stoul(response.substr(1, header_end - 1)));
        const auto body_start = header_end + 2;
        if (response.size() < body_start + length) {
            return false;
        }
        payload = response.substr(body_start, length);
        return true;
    } catch (...) {
        return false;
    }
#endif
}

size_t SlaveReplication::applyPayload(const std::string& payload, uint64_t& last_offset) {
    std::istringstream input(payload);
    std::string line;
    std::vector<WalRecord> records;
    uint64_t fallback_offset = last_offset + 1;

    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        WalRecord record;
        if (parseWalRecordLine(line, fallback_offset, record)) {
            records.push_back(record);
            fallback_offset = std::max(fallback_offset, record.offset + 1);
        }
    }

    WalReplayer replayer(store_);
    size_t applied = 0;
    for (const auto& record : records) {
        if (replayer.replayCommand(record.command)) {
            last_offset = record.offset;
            ++applied;
        }
    }
    return applied;
}

}  // namespace lightkv
