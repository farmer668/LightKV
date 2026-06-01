#include "lightkv/replication/MasterReplication.h"

#include <sstream>

namespace lightkv {

MasterReplication::MasterReplication(Wal& wal) : wal_(wal) {}

std::string MasterReplication::syncPayloadAfter(uint64_t offset) const {
    std::ostringstream out;
    const auto records = wal_.loadRecordsAfter(offset);
    for (const auto& record : records) {
        out << formatWalRecord(record) << '\n';
    }
    return out.str();
}

}  // namespace lightkv
