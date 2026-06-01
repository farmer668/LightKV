#include "lightkv/persistence/Wal.h"

#include "lightkv/persistence/WalReplayer.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <system_error>
#include <utility>

namespace lightkv {

Wal::Wal(std::string path) : path_(std::move(path)) {}

bool Wal::open() {
    const auto parent = std::filesystem::path(path_).parent_path();
    if (!parent.empty()) {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec) {
            std::cerr << "failed to create WAL directory: " << parent.string() << '\n';
            return false;
        }
    }

    last_offset_ = 0;
    for (const auto& record : loadRecords()) {
        last_offset_ = std::max(last_offset_, record.offset);
    }

    stream_.open(path_, std::ios::out | std::ios::app);
    return stream_.is_open();
}

void Wal::close() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

uint64_t Wal::appendSet(const std::string& key, const std::string& value) {
    return appendRecord("SET " + key + " " + value);
}

uint64_t Wal::appendDel(const std::string& key) {
    return appendRecord("DEL " + key);
}

uint64_t Wal::appendExpire(const std::string& key, int seconds) {
    return appendRecord("EXPIRE " + key + " " + std::to_string(seconds));
}

std::vector<WalRecord> Wal::loadRecords() const {
    std::ifstream input(path_);
    std::vector<WalRecord> records;
    if (!input.is_open()) {
        return records;
    }

    std::string line;
    uint64_t fallback_offset = 1;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        WalRecord record;
        if (parseWalRecordLine(line, fallback_offset, record)) {
            records.push_back(record);
            fallback_offset = std::max(fallback_offset, record.offset + 1);
        } else {
            std::cerr << "skip invalid WAL record: " << line << '\n';
        }
    }

    return records;
}

std::vector<WalRecord> Wal::loadRecordsAfter(uint64_t offset) const {
    std::vector<WalRecord> result;
    for (const auto& record : loadRecords()) {
        if (record.offset > offset) {
            result.push_back(record);
        }
    }
    return result;
}

uint64_t Wal::lastOffset() const {
    return last_offset_;
}

const std::string& Wal::path() const {
    return path_;
}

size_t Wal::recordsWritten() const {
    return records_written_;
}

uint64_t Wal::appendRecord(const std::string& record) {
    if (!stream_.is_open()) {
        return 0;
    }

    const auto next_offset = last_offset_ + 1;
    stream_ << next_offset << '|' << record << '\n';
    stream_.flush();
    if (!stream_) {
        return 0;
    }

    last_offset_ = next_offset;
    ++records_written_;
    return next_offset;
}

std::string formatWalRecord(const WalRecord& record) {
    return std::to_string(record.offset) + "|" + record.command;
}

bool parseWalRecordLine(const std::string& line, uint64_t fallback_offset, WalRecord& record) {
    const auto pipe = line.find('|');
    if (pipe == std::string::npos) {
        record.offset = fallback_offset;
        record.command = line;
        return !record.command.empty();
    }

    if (pipe == 0) {
        return false;
    }

    try {
        std::size_t parsed = 0;
        const auto offset = std::stoull(line.substr(0, pipe), &parsed, 10);
        if (parsed != pipe || offset == 0) {
            return false;
        }
        record.offset = static_cast<uint64_t>(offset);
        record.command = line.substr(pipe + 1);
        return !record.command.empty();
    } catch (...) {
        return false;
    }
}

size_t replayWalRecords(const std::vector<WalRecord>& records, KVStore& store) {
    WalReplayer replayer(store);
    size_t applied = 0;
    for (const auto& record : records) {
        if (replayer.replayCommand(record.command)) {
            ++applied;
        }
    }
    return applied;
}

size_t replayWalRecords(const std::vector<std::string>& records, KVStore& store) {
    std::vector<WalRecord> wal_records;
    uint64_t offset = 1;
    for (const auto& record : records) {
        WalRecord wal_record;
        if (parseWalRecordLine(record, offset, wal_record)) {
            wal_records.push_back(wal_record);
            offset = std::max(offset, wal_record.offset + 1);
        }
    }
    return replayWalRecords(wal_records, store);
}

size_t replayWalFile(const Wal& wal, KVStore& store) {
    return replayWalRecords(wal.loadRecords(), store);
}

}  // namespace lightkv
