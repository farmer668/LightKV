#include "lightkv/common/Metrics.h"

#include <sstream>

namespace lightkv {

void Metrics::incTotalCommands() {
    ++total_commands_;
}

void Metrics::incPingCommands() {
    ++ping_commands_;
}

void Metrics::incGetCommands() {
    ++get_commands_;
}

void Metrics::incSetCommands() {
    ++set_commands_;
}

void Metrics::incDelCommands() {
    ++del_commands_;
}

void Metrics::incExistsCommands() {
    ++exists_commands_;
}

void Metrics::incExpireCommands() {
    ++expire_commands_;
}

void Metrics::incTtlCommands() {
    ++ttl_commands_;
}

void Metrics::incInfoCommands() {
    ++info_commands_;
}

void Metrics::incHits() {
    ++hits_;
}

void Metrics::incMisses() {
    ++misses_;
}

void Metrics::incConnection() {
    ++total_connections_;
    ++current_connections_;
}

void Metrics::decConnection() {
    size_t current = current_connections_.load();
    while (current > 0 && !current_connections_.compare_exchange_weak(current, current - 1)) {
    }
}

size_t Metrics::totalCommands() const {
    return total_commands_.load();
}

size_t Metrics::hits() const {
    return hits_.load();
}

size_t Metrics::misses() const {
    return misses_.load();
}

size_t Metrics::totalConnections() const {
    return total_connections_.load();
}

size_t Metrics::currentConnections() const {
    return current_connections_.load();
}

std::string Metrics::toString() const {
    std::ostringstream out;
    out << "total_commands:" << total_commands_.load() << '\n';
    out << "ping_commands:" << ping_commands_.load() << '\n';
    out << "get_commands:" << get_commands_.load() << '\n';
    out << "set_commands:" << set_commands_.load() << '\n';
    out << "del_commands:" << del_commands_.load() << '\n';
    out << "exists_commands:" << exists_commands_.load() << '\n';
    out << "expire_commands:" << expire_commands_.load() << '\n';
    out << "ttl_commands:" << ttl_commands_.load() << '\n';
    out << "info_commands:" << info_commands_.load() << '\n';
    out << "hits:" << hits_.load() << '\n';
    out << "misses:" << misses_.load() << '\n';
    out << "connected_clients:" << current_connections_.load() << '\n';
    out << "total_connections:" << total_connections_.load() << '\n';
    out << "current_connections:" << current_connections_.load();
    return out.str();
}

}  // namespace lightkv

