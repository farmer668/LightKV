#pragma once

#include "lightkv/cluster/NodeInfo.h"

#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace lightkv {

class ConsistentHash {
public:
    explicit ConsistentHash(size_t default_virtual_nodes = 100);

    void addNode(const NodeInfo& node);
    void addNode(const NodeInfo& node, size_t virtual_nodes);
    void removeNode(const std::string& node_id);
    std::optional<NodeInfo> getNode(const std::string& key) const;

    size_t ringSize() const;
    size_t realNodeCount() const;
    bool empty() const;

private:
    size_t hashKey(const std::string& key) const;

    size_t default_virtual_nodes_;
    std::map<size_t, NodeInfo> ring_;
    std::unordered_map<std::string, std::vector<size_t>> node_hashes_;
};

}  // namespace lightkv
