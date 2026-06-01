#pragma once

#include "lightkv/storage/KVStore.h"

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

namespace lightkv {

class Wal {
public:
    explicit Wal(std::string path);

    bool open();
    void close();

    bool appendSet(const std::string& key, const std::string& value);
    bool appendDel(const std::string& key);
    bool appendExpire(const std::string& key, int seconds);

    std::vector<std::string> loadRecords() const;

    const std::string& path() const;
    size_t recordsWritten() const;

private:
    bool appendRecord(const std::string& record);

    std::string path_;
    std::ofstream stream_;
    size_t records_written_ = 0;
};

size_t replayWalRecords(const std::vector<std::string>& records, KVStore& store);
size_t replayWalFile(const Wal& wal, KVStore& store);

}  // namespace lightkv

