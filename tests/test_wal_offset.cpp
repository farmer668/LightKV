#include "lightkv/persistence/Wal.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace {

const std::string kWalPath = "data/test_wal_offset_stage8.wal";

void removeWal() {
    std::error_code ec;
    std::filesystem::remove(kWalPath, ec);
}

}  // namespace

int main() {
    removeWal();

    {
        lightkv::Wal wal(kWalPath);
        assert(wal.open());
        assert(wal.appendSet("a", "1") == 1);
        assert(wal.appendExpire("a", 10) == 2);
        assert(wal.appendDel("a") == 3);
        assert(wal.lastOffset() == 3);
        wal.close();
    }

    {
        lightkv::Wal wal(kWalPath);
        const auto records = wal.loadRecords();
        assert(records.size() == 3);
        assert(records[0].offset == 1);
        assert(records[0].command == "SET a 1");
        assert(records[2].offset == 3);
        assert(records[2].command == "DEL a");

        const auto after_one = wal.loadRecordsAfter(1);
        assert(after_one.size() == 2);
        assert(after_one[0].offset == 2);
        assert(after_one[1].offset == 3);

        assert(wal.open());
        assert(wal.lastOffset() == 3);
        assert(wal.appendSet("b", "2") == 4);
        wal.close();
    }

    {
        removeWal();
        std::ofstream output(kWalPath);
        output << "SET legacy value\n";
        output << "DEL legacy\n";
        output.close();

        lightkv::Wal wal(kWalPath);
        const auto records = wal.loadRecords();
        assert(records.size() == 2);
        assert(records[0].offset == 1);
        assert(records[0].command == "SET legacy value");
        assert(records[1].offset == 2);
        assert(records[1].command == "DEL legacy");
    }

    removeWal();
    return 0;
}
