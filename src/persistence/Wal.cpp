#include "lightkv/persistence/Wal.h"

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

    stream_.open(path_, std::ios::out | std::ios::app);
    return stream_.is_open();
}

void Wal::close() {
    if (stream_.is_open()) {
        stream_.close();
    }
}

bool Wal::appendSet(const std::string& key, const std::string& value) {
    return appendRecord("SET " + key + " " + value);
}

bool Wal::appendDel(const std::string& key) {
    return appendRecord("DEL " + key);
}

bool Wal::appendExpire(const std::string& key, int seconds) {
    return appendRecord("EXPIRE " + key + " " + std::to_string(seconds));
}

std::vector<std::string> Wal::loadRecords() const {
    std::ifstream input(path_);
    std::vector<std::string> records;
    if (!input.is_open()) {
        return records;
    }

    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            records.push_back(line);
        }
    }

    return records;
}

const std::string& Wal::path() const {
    return path_;
}

size_t Wal::recordsWritten() const {
    return records_written_;
}

bool Wal::appendRecord(const std::string& record) {
    if (!stream_.is_open()) {
        return false;
    }

    stream_ << record << '\n';
    stream_.flush();
    if (!stream_) {
        return false;
    }

    ++records_written_;
    return true;
}

size_t replayWalRecords(const std::vector<std::string>& records, KVStore& store) {
    size_t applied = 0;

    for (const auto& record : records) {
        std::istringstream stream(record);
        std::string command;
        stream >> command;

        if (command == "SET") {
            std::string key;
            std::string value;
            std::string extra;
            if ((stream >> key >> value) && !(stream >> extra)) {
                store.set(key, value);
                ++applied;
            } else {
                std::cerr << "skip invalid WAL record: " << record << '\n';
            }
            continue;
        }

        if (command == "DEL") {
            std::string key;
            std::string extra;
            if ((stream >> key) && !(stream >> extra)) {
                store.del(key);
                ++applied;
            } else {
                std::cerr << "skip invalid WAL record: " << record << '\n';
            }
            continue;
        }

        if (command == "EXPIRE") {
            std::string key;
            std::string seconds_text;
            std::string extra;
            if ((stream >> key >> seconds_text) && !(stream >> extra)) {
                try {
                    std::size_t parsed = 0;
                    const int seconds = std::stoi(seconds_text, &parsed, 10);
                    if (parsed == seconds_text.size()) {
                        store.expire(key, seconds);
                        ++applied;
                    } else {
                        std::cerr << "skip invalid WAL record: " << record << '\n';
                    }
                } catch (...) {
                    std::cerr << "skip invalid WAL record: " << record << '\n';
                }
            } else {
                std::cerr << "skip invalid WAL record: " << record << '\n';
            }
            continue;
        }

        std::cerr << "skip invalid WAL record: " << record << '\n';
    }

    return applied;
}

size_t replayWalFile(const Wal& wal, KVStore& store) {
    return replayWalRecords(wal.loadRecords(), store);
}

}  // namespace lightkv
