#include "lightkv/replication/ReplicationState.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace lightkv {

namespace {

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

}  // namespace

ReplicationRole parseReplicationRole(const std::string& value) {
    return lower(value) == "slave" ? ReplicationRole::Slave : ReplicationRole::Master;
}

std::string replicationRoleToString(ReplicationRole role) {
    return role == ReplicationRole::Slave ? "slave" : "master";
}

ReplicationState::ReplicationState() = default;

void ReplicationState::setRole(ReplicationRole role) {
    std::lock_guard<std::mutex> lock(mutex_);
    role_ = role;
}

ReplicationRole ReplicationState::role() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return role_;
}

void ReplicationState::setMaster(std::string host, int port) {
    std::lock_guard<std::mutex> lock(mutex_);
    master_host_ = std::move(host);
    master_port_ = port;
}

std::string ReplicationState::masterHost() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return master_host_;
}

int ReplicationState::masterPort() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return master_port_;
}

void ReplicationState::setReplicationOffset(uint64_t offset) {
    std::lock_guard<std::mutex> lock(mutex_);
    replication_offset_ = offset;
}

uint64_t ReplicationState::replicationOffset() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return replication_offset_;
}

void ReplicationState::recordSync(uint64_t offset, size_t records, std::string status, bool success) {
    std::lock_guard<std::mutex> lock(mutex_);
    replication_offset_ = offset;
    last_sync_records_ = records;
    last_sync_status_ = std::move(status);
    ++total_syncs_;
    if (!success) {
        ++failed_syncs_;
    }
}

size_t ReplicationState::lastSyncRecords() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_sync_records_;
}

std::string ReplicationState::lastSyncStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_sync_status_;
}

size_t ReplicationState::totalSyncs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return total_syncs_;
}

size_t ReplicationState::failedSyncs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return failed_syncs_;
}

std::string ReplicationState::info(uint64_t master_offset) const {
    std::lock_guard<std::mutex> lock(mutex_);
    const auto offset = role_ == ReplicationRole::Master ? master_offset : replication_offset_;

    std::ostringstream out;
    out << "role:" << replicationRoleToString(role_) << '\n';
    out << "master_host:" << master_host_ << '\n';
    out << "master_port:" << master_port_ << '\n';
    out << "replication_offset:" << offset << '\n';
    out << "last_sync_records:" << last_sync_records_ << '\n';
    out << "last_sync_status:" << last_sync_status_ << '\n';
    out << "total_syncs:" << total_syncs_ << '\n';
    out << "failed_syncs:" << failed_syncs_;
    return out.str();
}

}  // namespace lightkv
