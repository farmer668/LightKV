#include <iostream>
#include <cstddef>
#include <string>

#ifdef LIGHTKV_ENABLE_TCP_SERVER
#include "lightkv/net/TcpServer.h"
#endif

namespace {

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [--host HOST] [--port PORT] [--max-keys COUNT]\n";
}

bool parseSizeArg(const std::string& text, size_t& value) {
    try {
        std::size_t parsed = 0;
        const unsigned long long parsed_value = std::stoull(text, &parsed, 10);
        if (parsed != text.size()) {
            return false;
        }
        value = static_cast<size_t>(parsed_value);
        return true;
    } catch (...) {
        return false;
    }
}

bool parseArgs(int argc, char* argv[], std::string& host, int& port, size_t& max_keys) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--host") {
            if (i + 1 >= argc) {
                return false;
            }
            host = argv[++i];
            continue;
        }

        if (arg == "--port") {
            if (i + 1 >= argc) {
                return false;
            }
            try {
                std::size_t parsed = 0;
                const std::string port_text = argv[++i];
                port = std::stoi(port_text, &parsed, 10);
                if (parsed != port_text.size()) {
                    return false;
                }
            } catch (...) {
                return false;
            }
            if (port <= 0 || port > 65535) {
                return false;
            }
            continue;
        }

        if (arg == "--max-keys") {
            if (i + 1 >= argc || !parseSizeArg(argv[++i], max_keys)) {
                return false;
            }
            continue;
        }

        return false;
    }

    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    std::cout << "LightKV server starting..." << '\n';
    std::cout << "Stage 5 - LRU eviction" << '\n';

    std::string host = "127.0.0.1";
    int port = 6379;
    size_t max_keys = 10000;
    if (!parseArgs(argc, argv, host, port, max_keys)) {
        printUsage(argv[0]);
        return 1;
    }

#ifdef LIGHTKV_ENABLE_TCP_SERVER
    std::cout << "Platform: Linux/Unix target build" << '\n';

    lightkv::TcpServer server(host, port, max_keys);
    if (!server.start()) {
        return 1;
    }

    std::cout << "Listening on " << host << ":" << port << '\n';
    server.run();
    return 0;
#else
#if defined(_WIN32)
    std::cout << "Platform: Windows scaffold build" << '\n';
#else
    std::cout << "Platform: Unknown target build" << '\n';
#endif
    std::cout << "TCP server is only available on Linux/Unix." << '\n';
    return 0;
#endif
}
