#pragma once

#include <string>

namespace lightkv {

struct NodeInfo {
    std::string id;
    std::string host;
    int port = 0;
};

inline bool operator==(const NodeInfo& lhs, const NodeInfo& rhs) {
    return lhs.id == rhs.id && lhs.host == rhs.host && lhs.port == rhs.port;
}

}  // namespace lightkv
