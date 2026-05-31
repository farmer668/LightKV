#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "LightKV CLI - Stage 2" << '\n';
    std::cout << "Type QUIT to exit." << '\n';

    lightkv::KVStore store;
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

