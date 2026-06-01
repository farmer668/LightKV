#include "lightkv/common/Config.h"
#include "lightkv/common/Logger.h"
#include "lightkv/common/Metrics.h"
#include "lightkv/persistence/Wal.h"
#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cstddef>
#include <iostream>
#include <string>

namespace {

struct CliOptions {
    size_t max_keys = 10000;
    bool wal_enabled = true;
    std::string wal_path = "data/lightkv_cli.wal";
    std::string log_level = "INFO";
};

struct CliOverrides {
    bool max_keys = false;
    bool wal_enabled = false;
    bool wal_path = false;
    bool log_level = false;
};

void printUsage(const char* program) {
    std::cerr << "Usage: " << program
              << " [--config PATH] [--max-keys COUNT]"
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

bool parseCommandLine(
    int argc,
    char* argv[],
    CliOptions& cli,
    CliOverrides& overrides,
    std::string& config_path,
    bool& has_config) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--config") {
            if (i + 1 >= argc) {
                return false;
            }
            config_path = argv[++i];
            has_config = true;
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

CliOptions loadConfigOptions(const std::string& config_path, bool has_config) {
    CliOptions options;
    if (!has_config) {
        return options;
    }

    lightkv::Config config;
    config.loadFromFile(config_path);
    options.max_keys = config.getSizeT("max_keys", options.max_keys);
    options.wal_enabled = config.getBool("wal_enabled", options.wal_enabled);
    options.wal_path = config.getString("wal_path", options.wal_path);
    options.log_level = config.getString("log_level", options.log_level);
    return options;
}

void applyOverrides(CliOptions& options, const CliOptions& cli, const CliOverrides& overrides) {
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
    CliOptions cli_options;
    CliOverrides overrides;
    std::string config_path;
    bool has_config = false;
    if (!parseCommandLine(argc, argv, cli_options, overrides, config_path, has_config)) {
        printUsage(argv[0]);
        return 1;
    }

    auto options = loadConfigOptions(config_path, has_config);
    applyOverrides(options, cli_options, overrides);
    lightkv::Logger::instance().setLevel(lightkv::parseLogLevel(options.log_level));

    std::cout << "LightKV CLI - Stage 7" << '\n';
    std::cout << "Type QUIT to exit." << '\n';

    lightkv::KVStore store(options.max_keys);
    lightkv::Wal wal(options.wal_path);
    if (options.wal_enabled) {
        lightkv::replayWalFile(wal, store);
        if (!wal.open()) {
            lightkv::Logger::instance().error("failed to open WAL: " + options.wal_path);
            return 1;
        }
    }

    lightkv::Parser parser;
    lightkv::Metrics metrics;
    lightkv::CommandExecutor executor(
        store,
        options.wal_enabled ? &wal : nullptr,
        options.wal_enabled,
        options.wal_path,
        &metrics);

    std::string line;
    while (true) {
        std::cout << "lightkv> " << std::flush;
        if (!std::getline(std::cin, line)) {
            break;
        }

        const auto command = parser.parseLine(line);
        const auto response = executor.execute(command);
        std::cout << response;

        if (command.type == lightkv::CommandType::Quit) {
            break;
        }
    }

    wal.close();
    return 0;
}
