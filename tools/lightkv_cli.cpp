#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cstddef>
#include <iostream>
#include <string>

namespace {

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [--max-keys COUNT]\n";
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

bool parseArgs(int argc, char* argv[], size_t& max_keys) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
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
    size_t max_keys = 10000;
    if (!parseArgs(argc, argv, max_keys)) {
        printUsage(argv[0]);
        return 1;
    }

    std::cout << "LightKV CLI - Stage 5" << '\n';
    std::cout << "Type QUIT to exit." << '\n';

    lightkv::KVStore store(max_keys);
    lightkv::Parser parser;
    lightkv::CommandExecutor executor(store);

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

    return 0;
}
