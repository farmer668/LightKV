#include "lightkv/client/ClusterClient.h"
#include "lightkv/cluster/NodeInfo.h"

#include <cassert>
#include <string>

int main() {
    lightkv::ClusterClient client(10);
    assert(!client.route("a").has_value());
    assert(client.get("a") == "-ERR no available node\r\n");

    lightkv::NodeInfo node1{"node-1", "127.0.0.1", 6379};
    lightkv::NodeInfo node2{"node-2", "127.0.0.1", 6380};
    lightkv::NodeInfo node3{"node-3", "127.0.0.1", 6381};

    client.addNode(node1);
    client.addNode(node2);
    client.addNode(node3);

    const auto route_a = client.route("a");
    assert(route_a.has_value());
    const auto route_a_again = client.route("a");
    assert(route_a_again.has_value());
    assert(route_a_again->id == route_a->id);

    client.removeNode(route_a->id);
    const auto route_after_remove = client.route("a");
    assert(route_after_remove.has_value());
    assert(route_after_remove->id != route_a->id);

    client.removeNode("node-1");
    client.removeNode("node-2");
    client.removeNode("node-3");
    assert(!client.route("a").has_value());

    return 0;
}
