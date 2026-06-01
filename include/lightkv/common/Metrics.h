#pragma once

#include <atomic>
#include <cstddef>
#include <string>

namespace lightkv {

class Metrics {
public:
    void incTotalCommands();
    void incPingCommands();
    void incGetCommands();
    void incSetCommands();
    void incDelCommands();
    void incExistsCommands();
    void incExpireCommands();
    void incTtlCommands();
    void incInfoCommands();
    void incHits();
    void incMisses();
    void incConnection();
    void decConnection();

    size_t totalCommands() const;
    size_t hits() const;
    size_t misses() const;
    size_t totalConnections() const;
    size_t currentConnections() const;

    std::string toString() const;

private:
    std::atomic<size_t> total_commands_{0};
    std::atomic<size_t> ping_commands_{0};
    std::atomic<size_t> get_commands_{0};
    std::atomic<size_t> set_commands_{0};
    std::atomic<size_t> del_commands_{0};
    std::atomic<size_t> exists_commands_{0};
    std::atomic<size_t> expire_commands_{0};
    std::atomic<size_t> ttl_commands_{0};
    std::atomic<size_t> info_commands_{0};
    std::atomic<size_t> hits_{0};
    std::atomic<size_t> misses_{0};
    std::atomic<size_t> total_connections_{0};
    std::atomic<size_t> current_connections_{0};
};

}  // namespace lightkv

