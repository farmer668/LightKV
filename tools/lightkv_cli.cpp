#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/persistence/Wal.h"
#include "lightkv/storage/KVStore.h"

#include <cstddef>
#include <iostream>
#include <string>

namespace {

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [--max-keys COUNT] [--wal-path PATH] [--disable-wal]\n";
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

bool parseArgs(int argc, char* argv[], size_t& max_keys, bool& wal_enabled, std::string& wal_path) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--max-keys") {
            if (i + 1 >= argc || !parseSizeArg(argv[++i], max_keys)) {
                return false;
            }
            continue;
        }
        if (arg == "--wal-path") {
            if (i + 1 >= argc) {
                return false;
            }
            wal_path = argv[++i];
            continue;
        }
        if (arg == "--disable-wal") {
            wal_enabled = false;
            continue;
        }
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    size_t max_keys = 10000;
    bool wal_enabled = true;
    std::string wal_path = "data/lightkv_cli.wal";
    if (!parseArgs(argc, argv, max_keys, wal_enabled, wal_path)) {
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "LightKV CLI - Stage 6" << '\n';
    std::cout << "Type QUIT to exit." << '\n';

    lightkv::KVStore store(max_keys);
    lightkv::Wal wal(wal_path);
    if (wal_enabled) {
        lightkv::replayWalFile(wal, store);
        if (!wal.open()) {
            std::cerr << "failed to open WAL: " << wal_path << '\n';
            return 1;
        }
    }

    lightkv::Parser parser;
    lightkv::CommandExecutor executor(
        store,
        wal_enabled ? &wal : nullptr,
        wal_enabled,
        wal_path);

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
