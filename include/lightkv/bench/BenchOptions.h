#pragma once

#include <string>

namespace lightkv {

struct BenchOptions {
    std::string host = "127.0.0.1";
    int port = 6379;
    int clients = 10;
    int requests = 1000;
    int value_size = 16;
    double read_ratio = 0.8;
};

bool parseBenchOptions(int argc, char** argv, BenchOptions& options);

}  // namespace lightkv
