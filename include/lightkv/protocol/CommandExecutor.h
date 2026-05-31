#pragma once

#include "lightkv/protocol/Command.h"
#include "lightkv/storage/KVStore.h"

#include <string>

namespace lightkv {

class CommandExecutor {
public:
    explicit CommandExecutor(KVStore& store);

    std::string execute(const Command& command);

private:
    KVStore& store_;
};

}  // namespace lightkv

