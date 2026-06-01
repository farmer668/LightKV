#pragma once

#include "lightkv/cluster/ConsistentHash.h"
#include "lightkv/cluster/NodeInfo.h"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace lightkv {

class ClusterClient {
public:
    explicit ClusterClient(size_t virtual_nodes = 100);

    void addNode(const NodeInfo& node);
    void removeNode(const std::string& node_id);

    std::optional<NodeInfo> route(const std::string& key) const;

    std::string set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    std::string del(const std::string& key);
    std::string expire(const std::string& key, int seconds);
    std::string ttl(const std::string& key);

    std::string infoAll();

private:
    std::string sendToKeyNode(const std::string& key, const std::string& command);

    size_t virtual_nodes_;
    ConsistentHash hash_;
    std::unordered_map<std::string, NodeInfo> nodes_;
};

}  // namespace lightkv
