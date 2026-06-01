#pragma once

#include "lightkv/storage/KVStore.h"

#include <cstdint>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

namespace lightkv {

struct WalRecord {
    uint64_t offset = 0;
    std::string command;
};

class Wal {
public:
    explicit Wal(std::string path);

    bool open();
    void close();

    uint64_t appendSet(const std::string& key, const std::string& value);
    uint64_t appendDel(const std::string& key);
    uint64_t appendExpire(const std::string& key, int seconds);

    std::vector<WalRecord> loadRecords() const;
    std::vector<WalRecord> loadRecordsAfter(uint64_t offset) const;

    uint64_t lastOffset() const;
    const std::string& path() const;
    size_t recordsWritten() const;

private:
    uint64_t appendRecord(const std::string& record);

    std::string path_;
    std::ofstream stream_;
    uint64_t last_offset_ = 0;
    size_t records_written_ = 0;
};

std::string formatWalRecord(const WalRecord& record);
bool parseWalRecordLine(const std::string& line, uint64_t fallback_offset, WalRecord& record);

size_t replayWalRecords(const std::vector<WalRecord>& records, KVStore& store);
size_t replayWalRecords(const std::vector<std::string>& records, KVStore& store);
size_t replayWalFile(const Wal& wal, KVStore& store);

}  // namespace lightkv
