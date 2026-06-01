#include "lightkv/cluster/ConsistentHash.h"

#include <functional>

namespace lightkv {

ConsistentHash::ConsistentHash(size_t default_virtual_nodes)
    : default_virtual_nodes_(default_virtual_nodes) {}

void ConsistentHash::addNode(const NodeInfo& node) {
    addNode(node, default_virtual_nodes_);
}

void ConsistentHash::addNode(const NodeInfo& node, size_t virtual_nodes) {
    if (node.id.empty() || virtual_nodes == 0) {
        return;
    }

    removeNode(node.id);

    std::vector<size_t> hashes;
    hashes.reserve(virtual_nodes);
    for (size_t i = 0; i < virtual_nodes; ++i) {
        auto hash = hashKey(node.id + "#" + std::to_string(i));
        while (ring_.find(hash) != ring_.end()) {
            ++hash;
        }
        ring_[hash] = node;
        hashes.push_back(hash);
    }

    node_hashes_[node.id] = std::move(hashes);
}

void ConsistentHash::removeNode(const std::string& node_id) {
    const auto it = node_hashes_.find(node_id);
    if (it == node_hashes_.end()) {
        return;
    }

    for (const auto hash : it->second) {
        ring_.erase(hash);
    }
    node_hashes_.erase(it);
}

std::optional<NodeInfo> ConsistentHash::getNode(const std::string& key) const {
    if (ring_.empty()) {
        return std::nullopt;
    }

    const auto hash = hashKey(key);
    auto it = ring_.lower_bound(hash);
    if (it == ring_.end()) {
        it = ring_.begin();
    }
    return it->second;
}

size_t ConsistentHash::ringSize() const {
    return ring_.size();
}

size_t ConsistentHash::realNodeCount() const {
    return node_hashes_.size();
}

bool ConsistentHash::empty() const {
    return ring_.empty();
}

size_t ConsistentHash::hashKey(const std::string& key) const {
    return std::hash<std::string>{}(key);
}

}  // namespace lightkv
