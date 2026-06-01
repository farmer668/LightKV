#include "lightkv/persistence/Wal.h"
#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

namespace {

const std::string kWalPath = "data/test_wal_stage6.wal";

void removeTestWal() {
    std::error_code ec;
    std::filesystem::remove(kWalPath, ec);
}

std::string readAll(const std::string& path) {
    std::ifstream input(path);
    return std::string(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

}  // namespace

int main() {
    removeTestWal();

    {
        lightkv::Wal wal(kWalPath);
        assert(wal.open());
        assert(wal.appendSet("a", "1"));
        assert(wal.appendExpire("a", 10));
        assert(wal.appendDel("a"));
        assert(wal.recordsWritten() == 3);
        wal.close();

        assert(std::filesystem::exists(kWalPath));
        const auto content = readAll(kWalPath);
        assert(content.find("SET a 1\n") != std::string::npos);
        assert(content.find("EXPIRE a 10\n") != std::string::npos);
        assert(content.find("DEL a\n") != std::string::npos);

        const auto records = wal.loadRecords();
        assert(records.size() == 3);
        assert(records[0] == "SET a 1");
        assert(records[1] == "EXPIRE a 10");
        assert(records[2] == "DEL a");
    }

    {
        lightkv::KVStore store;
        const std::vector<std::string> records = {"SET persist hello"};
        assert(lightkv::replayWalRecords(records, store) == 1);
        const auto value = store.get("persist");
        assert(value.has_value());
        assert(value.value() == "hello");
    }

    {
        lightkv::KVStore store;
        const std::vector<std::string> records = {"SET gone value", "DEL gone"};
        assert(lightkv::replayWalRecords(records, store) == 2);
        assert(!store.get("gone").has_value());
    }

    {
        lightkv::KVStore store;
        const std::vector<std::string> records = {"SET temp value", "EXPIRE temp 10"};
        assert(lightkv::replayWalRecords(records, store) == 2);
        assert(store.ttl("temp") != -2);
        const auto value = store.get("temp");
        assert(value.has_value());
        assert(value.value() == "value");
    }

    {
        lightkv::KVStore store;
        const std::vector<std::string> records = {
            "SET ok yes",
            "BAD line",
            "SET missing_value",
            "EXPIRE ok nope"};
        assert(lightkv::replayWalRecords(records, store) == 1);
        const auto value = store.get("ok");
        assert(value.has_value());
        assert(value.value() == "yes");
    }

    {
        removeTestWal();
        lightkv::Wal writer(kWalPath);
        assert(writer.open());
        assert(writer.appendSet("x", "1"));
        writer.close();

        lightkv::Wal replayer(kWalPath);
        lightkv::KVStore store;
        assert(replayer.recordsWritten() == 0);
        assert(lightkv::replayWalFile(replayer, store) == 1);
        assert(replayer.recordsWritten() == 0);
        const auto records = replayer.loadRecords();
        assert(records.size() == 1);
        assert(records[0] == "SET x 1");
    }

    {
        removeTestWal();
        lightkv::KVStore store;
        lightkv::Wal wal(kWalPath);
        assert(wal.open());
        lightkv::Parser parser;
        lightkv::CommandExecutor executor(store, &wal, true, kWalPath);

        assert(executor.execute(parser.parseLine("SET walkey walvalue")) == "+OK\r\n");
        assert(executor.execute(parser.parseLine("EXPIRE walkey 10")) == ":1\r\n");
        assert(executor.execute(parser.parseLine("DEL walkey")) == ":1\r\n");
        wal.close();

        const auto records = wal.loadRecords();
        assert(records.size() == 3);
        assert(records[0] == "SET walkey walvalue");
        assert(records[1] == "EXPIRE walkey 10");
        assert(records[2] == "DEL walkey");
    }

    removeTestWal();
    return 0;
}
