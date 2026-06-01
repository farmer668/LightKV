#pragma once

#include "lightkv/persistence/Wal.h"
#include "lightkv/storage/KVStore.h"

#include <cstdint>
#include <string>
#include <vector>

namespace lightkv {

class WalReplayer {
public:
    explicit WalReplayer(KVStore& store);

    bool replayCommand(const std::string& command);
    uint64_t replayRecords(const std::vector<WalRecord>& records);

private:
    KVStore& store_;
};

}  // namespace lightkv
