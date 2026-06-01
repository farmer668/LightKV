#pragma once

#include "lightkv/persistence/WalReplayer.h"
#include "lightkv/replication/ReplicationState.h"
#include "lightkv/storage/KVStore.h"

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

namespace lightkv {

class SlaveReplication {
public:
    SlaveReplication(
        ReplicationState& state,
        KVStore& store,
        std::chrono::milliseconds interval);
    ~SlaveReplication();

    void start();
    void stop();
    bool syncOnce();

private:
    bool requestSyncPayload(uint64_t offset, std::string& payload);
    size_t applyPayload(const std::string& payload, uint64_t& last_offset);

    ReplicationState& state_;
    KVStore& store_;
    std::chrono::milliseconds interval_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};

}  // namespace lightkv
