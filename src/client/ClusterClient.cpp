#include "lightkv/client/ClusterClient.h"

#include "lightkv/client/LightKVClient.h"

#include <sstream>
#include <utility>

namespace lightkv {

ClusterClient::ClusterClient(size_t virtual_nodes)
    : virtual_nodes_(virtual_nodes), hash_(virtual_nodes) {}

void ClusterClient::addNode(const NodeInfo& node) {
    if (node.id.empty()) {
        return;
    }
    nodes_[node.id] = node;
    hash_.addNode(node, virtual_nodes_);
}

void ClusterClient::removeNode(const std::string& node_id) {
    nodes_.erase(node_id);
    hash_.removeNode(node_id);
}

std::optional<NodeInfo> ClusterClient::route(const std::string& key) const {
    return hash_.getNode(key);
}

std::string ClusterClient::set(const std::string& key, const std::string& value) {
    return sendToKeyNode(key, "SET " + key + " " + value);
}

std::string ClusterClient::get(const std::string& key) {
    return sendToKeyNode(key, "GET " + key);
}

std::string ClusterClient::del(const std::string& key) {
    return sendToKeyNode(key, "DEL " + key);
}

std::string ClusterClient::expire(const std::string& key, int seconds) {
    return sendToKeyNode(key, "EXPIRE " + key + " " + std::to_string(seconds));
}

std::string ClusterClient::ttl(const std::string& key) {
    return sendToKeyNode(key, "TTL " + key);
}

std::string ClusterClient::infoAll() {
    if (nodes_.empty()) {
        return "-ERR no available node\r\n";
    }

    std::ostringstream out;
    for (const auto& item : nodes_) {
        const auto& node = item.second;
        LightKVClient client(node.host, node.port);
        out << "# " << node.id << " " << node.host << ":" << node.port << '\n';
        out << client.info();
        if (out.tellp() > 0) {
            out << '\n';
        }
    }
    return out.str();
}

std::string ClusterClient::sendToKeyNode(const std::string& key, const std::string& command) {
    const auto node = route(key);
    if (!node.has_value()) {
        return "-ERR no available node\r\n";
    }

    LightKVClient client(node->host, node->port);
    return client.sendCommand(command);
}

}  // namespace lightkv
