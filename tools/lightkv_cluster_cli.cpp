#include "lightkv/client/ClusterClient.h"
#include "lightkv/cluster/NodeInfo.h"

#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::vector<lightkv::NodeInfo> nodes;
    size_t virtual_nodes = 100;
};

void printUsage(const char* program) {
    std::cerr << "Usage: " << program
              << " --nodes HOST:PORT[,HOST:PORT...] [--virtual-nodes COUNT]\n";
}

bool parseSizeArg(const std::string& text, size_t& value) {
    try {
        std::size_t parsed = 0;
        const auto parsed_value = std::stoull(text, &parsed, 10);
        if (parsed != text.size()) {
            return false;
        }
        value = static_cast<size_t>(parsed_value);
        return true;
    } catch (...) {
        return false;
    }
}

bool parseNode(const std::string& text, lightkv::NodeInfo& node) {
    const auto colon = text.rfind(':');
    if (colon == std::string::npos || colon == 0 || colon + 1 >= text.size()) {
        return false;
    }

    try {
        std::size_t parsed = 0;
        const auto port = std::stoi(text.substr(colon + 1), &parsed, 10);
        if (parsed != text.size() - colon - 1 || port <= 0 || port > 65535) {
            return false;
        }
        node.host = text.substr(0, colon);
        node.port = port;
        node.id = "node-" + node.host + ":" + std::to_string(node.port);
        return true;
    } catch (...) {
        return false;
    }
}

bool parseNodes(const std::string& text, std::vector<lightkv::NodeInfo>& nodes) {
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        lightkv::NodeInfo node;
        if (!parseNode(item, node)) {
            return false;
        }
        nodes.push_back(node);
    }
    return !nodes.empty();
}

bool parseCommandLine(int argc, char* argv[], Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--nodes") {
            if (i + 1 >= argc || !parseNodes(argv[++i], options.nodes)) {
                return false;
            }
            continue;
        }

        if (arg == "--virtual-nodes") {
            if (i + 1 >= argc || !parseSizeArg(argv[++i], options.virtual_nodes)) {
                return false;
            }
            continue;
        }

        return false;
    }

    return !options.nodes.empty();
}

std::vector<std::string> splitLine(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string upper(std::string value) {
    for (auto& ch : value) {
        if (ch >= 'a' && ch <= 'z') {
            ch = static_cast<char>(ch - 'a' + 'A');
        }
    }
    return value;
}

}  // namespace

int main(int argc, char* argv[]) {
    Options options;
    if (!parseCommandLine(argc, argv, options)) {
        printUsage(argv[0]);
        return 1;
    }

    lightkv::ClusterClient client(options.virtual_nodes);
    std::cout << "LightKV Cluster CLI - Stage 9\n";
    std::cout << "Nodes:\n";
    for (const auto& node : options.nodes) {
        client.addNode(node);
        std::cout << "  " << node.id << " " << node.host << ":" << node.port << '\n';
    }
    std::cout << "Type QUIT to exit.\n";

    std::string line;
    while (true) {
        std::cout << "lightkv-cluster> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }

        const auto tokens = splitLine(line);
        if (tokens.empty()) {
            continue;
        }

        const auto command = upper(tokens[0]);
        if (command == "QUIT") {
            std::cout << "+BYE\r\n";
            break;
        }

        if (command == "ROUTE") {
            if (tokens.size() != 2) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            const auto node = client.route(tokens[1]);
            if (!node.has_value()) {
                std::cout << "-ERR no available node\r\n";
                continue;
            }
            std::cout << "key " << tokens[1] << " -> node " << node->id << "\n";
            continue;
        }

        if (command == "INFO") {
            if (tokens.size() != 1) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            std::cout << client.infoAll();
            continue;
        }

        if (command == "SET") {
            if (tokens.size() != 3) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            std::cout << client.set(tokens[1], tokens[2]);
            continue;
        }

        if (command == "GET") {
            if (tokens.size() != 2) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            std::cout << client.get(tokens[1]);
            continue;
        }

        if (command == "DEL") {
            if (tokens.size() != 2) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            std::cout << client.del(tokens[1]);
            continue;
        }

        if (command == "EXPIRE") {
            if (tokens.size() != 3) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            try {
                std::size_t parsed = 0;
                const auto seconds = std::stoi(tokens[2], &parsed, 10);
                if (parsed != tokens[2].size()) {
                    std::cout << "-ERR invalid expire seconds\r\n";
                    continue;
                }
                std::cout << client.expire(tokens[1], seconds);
            } catch (...) {
                std::cout << "-ERR invalid expire seconds\r\n";
            }
            continue;
        }

        if (command == "TTL") {
            if (tokens.size() != 2) {
                std::cout << "-ERR wrong number of arguments\r\n";
                continue;
            }
            std::cout << client.ttl(tokens[1]);
            continue;
        }

        std::cout << "-ERR unknown command\r\n";
    }

    return 0;
}
