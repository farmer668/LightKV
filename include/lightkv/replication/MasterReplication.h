#pragma once

#include "lightkv/persistence/Wal.h"

#include <cstdint>
#include <string>

namespace lightkv {

class MasterReplication {
public:
    explicit MasterReplication(Wal& wal);

    std::string syncPayloadAfter(uint64_t offset) const;

private:
    Wal& wal_;
};

}  // namespace lightkv
