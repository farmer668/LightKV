#include "lightkv/common/Metrics.h"

#include <cassert>
#include <string>

int main() {
    lightkv::Metrics metrics;
    metrics.incTotalCommands();
    metrics.incGetCommands();
    metrics.incSetCommands();
    metrics.incHits();
    metrics.incMisses();
    metrics.incConnection();
    metrics.incConnection();
    metrics.decConnection();

    assert(metrics.totalCommands() == 1);
    assert(metrics.hits() == 1);
    assert(metrics.misses() == 1);
    assert(metrics.totalConnections() == 2);
    assert(metrics.currentConnections() == 1);

    const auto text = metrics.toString();
    assert(text.find("total_commands:1") != std::string::npos);
    assert(text.find("get_commands:1") != std::string::npos);
    assert(text.find("set_commands:1") != std::string::npos);
    assert(text.find("hits:1") != std::string::npos);
    assert(text.find("misses:1") != std::string::npos);
    assert(text.find("total_connections:2") != std::string::npos);
    assert(text.find("current_connections:1") != std::string::npos);

    return 0;
}

