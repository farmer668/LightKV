#pragma once

#include "lightkv/common/Metrics.h"
#include "lightkv/persistence/Wal.h"
#include "lightkv/protocol/Command.h"
#include "lightkv/replication/ReplicationState.h"
#include "lightkv/storage/KVStore.h"

#include <string>

namespace lightkv {

class CommandExecutor {
public:
    explicit CommandExecutor(
        KVStore& store,
        Wal* wal = nullptr,
        bool wal_enabled = false,
        std::string wal_path = "",
        Metrics* metrics = nullptr,
        ReplicationState* replication_state = nullptr);

    std::string execute(const Command& command);

private:
    KVStore& store_;
    Wal* wal_;
    bool wal_enabled_;
    std::string wal_path_;
    Metrics* metrics_;
    ReplicationState* replication_state_;
};

}  // namespace lightkv
