#pragma once

#include "lightkv/persistence/Wal.h"
#include "lightkv/protocol/Command.h"
#include "lightkv/storage/KVStore.h"

#include <string>

namespace lightkv {

class CommandExecutor {
public:
    explicit CommandExecutor(
        KVStore& store,
        Wal* wal = nullptr,
        bool wal_enabled = false,
        std::string wal_path = "");

    std::string execute(const Command& command);

private:
    KVStore& store_;
    Wal* wal_;
    bool wal_enabled_;
    std::string wal_path_;
};

}  // namespace lightkv
