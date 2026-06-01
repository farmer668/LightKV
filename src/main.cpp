#include "lightkv/common/Config.h"
#include "lightkv/common/Logger.h"

#include <cstddef>
#include <iostream>
#include <string>

#ifdef LIGHTKV_ENABLE_TCP_SERVER
#include "lightkv/net/TcpServer.h"
#endif

namespace {

struct ServerOptions {
    std::string host = "127.0.0.1";
    int port = 6379;
    size_t max_keys = 10000;
    bool wal_enabled = true;
    std::string wal_path = "data/lightkv.wal";
    std::string log_level = "INFO";
};

struct ServerOverrides {
    bool host = false;
    bool port = false;
    bool max_keys = false;
    bool wal_enabled = false;
    bool wal_path = false;
    bool log_level = false;
};

void printUsage(const char* program) {
    std::cerr << "Usage: " << program
              << " [--config PATH] [--host HOST] [--port PORT] [--max-keys COUNT]"
              << " [--wal-path PATH] [--disable-wal] [--log-level LEVEL]\n";
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

bool parsePortArg(const std::string& text, int& port) {
    try {
        std::size_t parsed = 0;
        port = std::stoi(text, &parsed, 10);
        return parsed == text.size() && port > 0 && port <= 65535;
    } catch (...) {
        return false;
    }
}

bool parseCommandLine(
    int argc,
    char* argv[],
    ServerOptions& cli,
    ServerOverrides& overrides,
    std::string& config_path) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--config") {
            if (i + 1 >= argc) {
                return false;
            }
            config_path = argv[++i];
            continue;
        }

        if (arg == "--host") {
            if (i + 1 >= argc) {
                return false;
            }
            cli.host = argv[++i];
            overrides.host = true;
            continue;
        }

        if (arg == "--port") {
            if (i + 1 >= argc || !parsePortArg(argv[++i], cli.port)) {
                return false;
            }
            overrides.port = true;
            continue;
        }

        if (arg == "--max-keys") {
            if (i + 1 >= argc || !parseSizeArg(argv[++i], cli.max_keys)) {
                return false;
            }
            overrides.max_keys = true;
            continue;
        }

        if (arg == "--wal-path") {
            if (i + 1 >= argc) {
                return false;
            }
            cli.wal_path = argv[++i];
            overrides.wal_path = true;
            continue;
        }

        if (arg == "--disable-wal") {
            cli.wal_enabled = false;
            overrides.wal_enabled = true;
            continue;
        }

        if (arg == "--log-level") {
            if (i + 1 >= argc) {
                return false;
            }
            cli.log_level = argv[++i];
            overrides.log_level = true;
            continue;
        }

        return false;
    }

    return true;
}

ServerOptions loadConfigOptions(const std::string& config_path) {
    ServerOptions options;
    lightkv::Config config;
    config.loadFromFile(config_path);

    options.host = config.getString("host", options.host);
    options.port = config.getInt("port", options.port);
    options.max_keys = config.getSizeT("max_keys", options.max_keys);
    options.wal_enabled = config.getBool("wal_enabled", options.wal_enabled);
    options.wal_path = config.getString("wal_path", options.wal_path);
    options.log_level = config.getString("log_level", options.log_level);
    return options;
}

void applyOverrides(ServerOptions& options, const ServerOptions& cli, const ServerOverrides& overrides) {
    if (overrides.host) {
        options.host = cli.host;
    }
    if (overrides.port) {
        options.port = cli.port;
    }
    if (overrides.max_keys) {
        options.max_keys = cli.max_keys;
    }
    if (overrides.wal_enabled) {
        options.wal_enabled = cli.wal_enabled;
    }
    if (overrides.wal_path) {
        options.wal_path = cli.wal_path;
    }
    if (overrides.log_level) {
        options.log_level = cli.log_level;
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    std::cout << "LightKV server starting..." << '\n';
    std::cout << "Stage 7 - config logger and metrics" << '\n';

    ServerOptions cli_options;
    ServerOverrides overrides;
    std::string config_path = "config/lightkv.example.conf";
    if (!parseCommandLine(argc, argv, cli_options, overrides, config_path)) {
        printUsage(argv[0]);
        return 1;
    }

    auto options = loadConfigOptions(config_path);
    applyOverrides(options, cli_options, overrides);
    lightkv::Logger::instance().setLevel(lightkv::parseLogLevel(options.log_level));
    lightkv::Logger::instance().info("LightKV server configuration loaded from " + config_path);

#ifdef LIGHTKV_ENABLE_TCP_SERVER
    std::cout << "Platform: Linux/Unix target build" << '\n';

    lightkv::TcpServer server(
        options.host,
        options.port,
        options.max_keys,
        options.wal_enabled,
        options.wal_path);
    if (!server.start()) {
        return 1;
    }

    std::cout << "Listening on " << options.host << ":" << options.port << '\n';
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
