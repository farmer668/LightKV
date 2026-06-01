#include "lightkv/bench/BenchOptions.h"

#include <cassert>
#include <initializer_list>
#include <vector>

namespace {

bool parse(std::initializer_list<const char*> args, lightkv::BenchOptions& options) {
    std::vector<char*> argv;
    argv.reserve(args.size());
    for (const auto* arg : args) {
        argv.push_back(const_cast<char*>(arg));
    }
    return lightkv::parseBenchOptions(static_cast<int>(argv.size()), argv.data(), options);
}

}  // namespace

int main() {
    {
        lightkv::BenchOptions options;
        assert(parse({"bench"}, options));
        assert(options.host == "127.0.0.1");
        assert(options.port == 6379);
        assert(options.clients == 10);
        assert(options.requests == 1000);
        assert(options.value_size == 16);
        assert(options.read_ratio == 0.8);
    }

    {
        lightkv::BenchOptions options;
        assert(parse({
            "bench",
            "--host", "0.0.0.0",
            "--port", "6380",
            "--clients", "4",
            "--requests", "200",
            "--value-size", "64",
            "--read-ratio", "0.25"}, options));
        assert(options.host == "0.0.0.0");
        assert(options.port == 6380);
        assert(options.clients == 4);
        assert(options.requests == 200);
        assert(options.value_size == 64);
        assert(options.read_ratio == 0.25);
    }

    {
        lightkv::BenchOptions options;
        assert(parse({"bench", "--read-ratio", "0"}, options));
        assert(options.read_ratio == 0.0);
        assert(parse({"bench", "--read-ratio", "1"}, options));
        assert(options.read_ratio == 1.0);
    }

    {
        lightkv::BenchOptions options;
        assert(!parse({"bench", "--read-ratio", "-0.1"}, options));
        assert(!parse({"bench", "--read-ratio", "1.1"}, options));
        assert(!parse({"bench", "--read-ratio", "abc"}, options));
        assert(!parse({"bench", "--value-size", "0"}, options));
        assert(!parse({"bench", "--clients", "0"}, options));
        assert(!parse({"bench", "--requests", "0"}, options));
        assert(!parse({"bench", "--port", "70000"}, options));
        assert(!parse({"bench", "--unknown"}, options));
    }

    return 0;
}
