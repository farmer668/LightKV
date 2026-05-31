#include "lightkv/storage/KVStore.h"

#include <iostream>

int main() {
    std::cout << "LightKV server starting..." << '\n';
    std::cout << "Stage 1 - in-memory KVStore" << '\n';

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

    return 0;
}
