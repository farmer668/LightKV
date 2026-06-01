#include "lightkv/cluster/ConsistentHash.h"
#include "lightkv/cluster/NodeInfo.h"

#include <cassert>
#include <set>
#include <string>

int main() {
    lightkv::ConsistentHash ring(10);
    assert(ring.empty());
    assert(ring.ringSize() == 0);
    assert(ring.realNodeCount() == 0);
    assert(!ring.getNode("a").has_value());

    lightkv::NodeInfo node1{"node-1", "127.0.0.1", 6379};
    ring.addNode(node1);
    assert(!ring.empty());
    assert(ring.ringSize() == 10);
    assert(ring.realNodeCount() == 1);

    const auto routed = ring.getNode("user:1");
    assert(routed.has_value());
    assert(routed->id == "node-1");

    const auto routed_again = ring.getNode("user:1");
    assert(routed_again.has_value());
    assert(routed_again->id == routed->id);

    lightkv::NodeInfo node2{"node-2", "127.0.0.1", 6380};
    lightkv::NodeInfo node3{"node-3", "127.0.0.1", 6381};
    ring.addNode(node2, 20);
    ring.addNode(node3, 30);
    assert(ring.ringSize() == 60);
    assert(ring.realNodeCount() == 3);

    std::set<std::string> seen_nodes;
    for (int i = 0; i < 200; ++i) {
        const auto node = ring.getNode("key:" + std::to_string(i));
        assert(node.has_value());
        seen_nodes.insert(node->id);
    }
    assert(seen_nodes.size() >= 2);

    ring.removeNode("node-2");
    assert(ring.ringSize() == 40);
    assert(ring.realNodeCount() == 2);
    for (int i = 0; i < 200; ++i) {
        const auto node = ring.getNode("key:" + std::to_string(i));
        assert(node.has_value());
        assert(node->id != "node-2");
    }

    ring.removeNode("node-1");
    ring.removeNode("node-3");
    assert(ring.empty());
    assert(!ring.getNode("anything").has_value());

    return 0;
}
