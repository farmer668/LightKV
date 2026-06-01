#pragma once

#include "lightkv/replication/ReplicationRole.h"

#include <cstdint>
#include <mutex>
#include <string>

namespace lightkv {

class ReplicationState {
public:
    ReplicationState();

    void setRole(ReplicationRole role);
    ReplicationRole role() const;

    void setMaster(std::string host, int port);
    std::string masterHost() const;
    int masterPort() const;

    void setReplicationOffset(uint64_t offset);
    uint64_t replicationOffset() const;

    void recordSync(uint64_t offset, size_t records, std::string status, bool success);
    size_t lastSyncRecords() const;
    std::string lastSyncStatus() const;
    size_t totalSyncs() const;
    size_t failedSyncs() const;

    std::string info(uint64_t master_offset = 0) const;

private:
    mutable std::mutex mutex_;
    ReplicationRole role_ = ReplicationRole::Master;
    std::string master_host_ = "127.0.0.1";
    int master_port_ = 6379;
    uint64_t replication_offset_ = 0;
    size_t last_sync_records_ = 0;
    std::string last_sync_status_ = "none";
    size_t total_syncs_ = 0;
    size_t failed_syncs_ = 0;
};

}  // namespace lightkv
