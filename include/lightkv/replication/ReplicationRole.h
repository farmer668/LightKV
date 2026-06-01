#pragma once

#include <string>

namespace lightkv {

enum class ReplicationRole {
    Master,
    Slave
};

ReplicationRole parseReplicationRole(const std::string& value);
std::string replicationRoleToString(ReplicationRole role);

}  // namespace lightkv
