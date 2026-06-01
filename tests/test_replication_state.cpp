#include "lightkv/replication/ReplicationState.h"

#include <cassert>
#include <string>

int main() {
    lightkv::ReplicationState state;
    assert(state.role() == lightkv::ReplicationRole::Master);
    assert(lightkv::replicationRoleToString(state.role()) == "master");

    state.setRole(lightkv::ReplicationRole::Slave);
    assert(state.role() == lightkv::ReplicationRole::Slave);
    assert(lightkv::parseReplicationRole("slave") == lightkv::ReplicationRole::Slave);
    assert(lightkv::parseReplicationRole("master") == lightkv::ReplicationRole::Master);

    state.setMaster("10.0.0.1", 6380);
    assert(state.masterHost() == "10.0.0.1");
    assert(state.masterPort() == 6380);

    state.setReplicationOffset(7);
    assert(state.replicationOffset() == 7);

    state.recordSync(9, 2, "OK", true);
    assert(state.replicationOffset() == 9);
    assert(state.lastSyncRecords() == 2);
    assert(state.lastSyncStatus() == "OK");
    assert(state.totalSyncs() == 1);
    assert(state.failedSyncs() == 0);

    state.recordSync(9, 0, "ERROR", false);
    assert(state.totalSyncs() == 2);
    assert(state.failedSyncs() == 1);

    const auto info = state.info();
    assert(info.find("role:slave") != std::string::npos);
    assert(info.find("master_host:10.0.0.1") != std::string::npos);
    assert(info.find("master_port:6380") != std::string::npos);
    assert(info.find("replication_offset:9") != std::string::npos);
    assert(info.find("last_sync_status:ERROR") != std::string::npos);

    return 0;
}
