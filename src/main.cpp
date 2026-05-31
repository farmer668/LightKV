#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <iostream>
#include <string>

int main() {
    std::cout << "LightKV server starting..." << '\n';
    std::cout << "Stage 2 - protocol parser and local CLI" << '\n';

#if defined(_WIN32)
    std::cout << "Platform: Windows scaffold build" << '\n';
#elif defined(__unix__) || defined(__APPLE__)
    std::cout << "Platform: Linux/Unix target build" << '\n';
#else
    std::cout << "Platform: Unknown target build" << '\n';
#endif

    lightkv::KVStore store;
    const auto status = store.set("stage", "1");
    const auto value = store.get("stage");

    if (status.ok() && value.has_value() && value.value() == "1") {
        std::cout << "KVStore self-check: OK" << '\n';
    } else {
        std::cout << "KVStore self-check: FAILED" << '\n';
    }

    lightkv::Parser parser;
    lightkv::CommandExecutor executor(store);
    const auto set_command = parser.parseLine("SET stage 2");
    const auto set_response = executor.execute(set_command);
    const auto get_command = parser.parseLine("GET stage");
    const auto get_response = executor.execute(get_command);

    if (set_response == "+OK\r\n" && get_response.find("2") != std::string::npos) {
        std::cout << "Protocol self-check: OK" << '\n';
    } else {
        std::cout << "Protocol self-check: FAILED" << '\n';
    }

    return 0;
}
