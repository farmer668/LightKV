#include "lightkv/persistence/WalReplayer.h"

#include <cstdint>
#include <iostream>
#include <sstream>

namespace lightkv {

WalReplayer::WalReplayer(KVStore& store) : store_(store) {}

bool WalReplayer::replayCommand(const std::string& command) {
    std::istringstream stream(command);
    std::string name;
    stream >> name;

    if (name == "SET") {
        std::string key;
        std::string value;
        std::string extra;
        if ((stream >> key >> value) && !(stream >> extra)) {
            store_.set(key, value);
            return true;
        }
        std::cerr << "skip invalid WAL record: " << command << '\n';
        return false;
    }

    if (name == "DEL") {
        std::string key;
        std::string extra;
        if ((stream >> key) && !(stream >> extra)) {
            store_.del(key);
            return true;
        }
        std::cerr << "skip invalid WAL record: " << command << '\n';
        return false;
    }

    if (name == "EXPIRE") {
        std::string key;
        std::string seconds_text;
        std::string extra;
        if ((stream >> key >> seconds_text) && !(stream >> extra)) {
            try {
                std::size_t parsed = 0;
                const int seconds = std::stoi(seconds_text, &parsed, 10);
                if (parsed == seconds_text.size()) {
                    store_.expire(key, seconds);
                    return true;
                }
            } catch (...) {
            }
        }
        std::cerr << "skip invalid WAL record: " << command << '\n';
        return false;
    }

    std::cerr << "skip invalid WAL record: " << command << '\n';
    return false;
}

uint64_t WalReplayer::replayRecords(const std::vector<WalRecord>& records) {
    uint64_t last_applied = 0;
    for (const auto& record : records) {
        if (replayCommand(record.command)) {
            last_applied = record.offset;
        }
    }
    return last_applied;
}

}  // namespace lightkv
