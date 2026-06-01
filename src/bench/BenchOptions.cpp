#include "lightkv/bench/BenchOptions.h"

#include <string>

namespace lightkv {

namespace {

bool parseInt(const std::string& text, int& value) {
    try {
        std::size_t parsed = 0;
        const auto parsed_value = std::stoi(text, &parsed, 10);
        if (parsed != text.size()) {
            return false;
        }
        value = parsed_value;
        return true;
    } catch (...) {
        return false;
    }
}

bool parseDouble(const std::string& text, double& value) {
    try {
        std::size_t parsed = 0;
        const auto parsed_value = std::stod(text, &parsed);
        if (parsed != text.size()) {
            return false;
        }
        value = parsed_value;
        return true;
    } catch (...) {
        return false;
    }
}

}  // namespace

bool parseBenchOptions(int argc, char** argv, BenchOptions& options) {
    options = BenchOptions{};

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--host") {
            if (i + 1 >= argc) {
                return false;
            }
            options.host = argv[++i];
            continue;
        }

        if (arg == "--port") {
            if (i + 1 >= argc || !parseInt(argv[++i], options.port)) {
                return false;
            }
            if (options.port <= 0 || options.port > 65535) {
                return false;
            }
            continue;
        }

        if (arg == "--clients") {
            if (i + 1 >= argc || !parseInt(argv[++i], options.clients)) {
                return false;
            }
            if (options.clients <= 0) {
                return false;
            }
            continue;
        }

        if (arg == "--requests") {
            if (i + 1 >= argc || !parseInt(argv[++i], options.requests)) {
                return false;
            }
            if (options.requests <= 0) {
                return false;
            }
            continue;
        }

        if (arg == "--value-size") {
            if (i + 1 >= argc || !parseInt(argv[++i], options.value_size)) {
                return false;
            }
            if (options.value_size <= 0) {
                return false;
            }
            continue;
        }

        if (arg == "--read-ratio") {
            if (i + 1 >= argc || !parseDouble(argv[++i], options.read_ratio)) {
                return false;
            }
            if (options.read_ratio < 0.0 || options.read_ratio > 1.0) {
                return false;
            }
            continue;
        }

        return false;
    }

    return true;
}

}  // namespace lightkv
