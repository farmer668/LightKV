#include "lightkv/bench/BenchOptions.h"
#include "lightkv/client/LightKVClient.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

void printUsage(const char* program) {
    std::cerr << "Usage: " << program
              << " [--host 127.0.0.1] [--port 6379] [--clients 10]"
              << " [--requests 1000] [--value-size 16] [--read-ratio 0.8]\n";
}

std::string makeValue(int size) {
    std::string value;
    value.reserve(static_cast<size_t>(size));
    for (int i = 0; i < size; ++i) {
        value.push_back(static_cast<char>('a' + (i % 26)));
    }
    return value;
}

bool isSuccess(const std::string& response) {
    return !response.empty() && response.rfind("-ERR", 0) != 0;
}

double percentile(std::vector<double>& values, double ratio) {
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const auto index = static_cast<size_t>((values.size() - 1) * ratio);
    return values[index];
}

void prewarm(const lightkv::BenchOptions& options, const std::string& value) {
    lightkv::LightKVClient client(options.host, options.port);
    for (int i = 0; i < options.requests; ++i) {
        client.set("bench:key:" + std::to_string(i), value);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    lightkv::BenchOptions options;
    if (!lightkv::parseBenchOptions(argc, argv, options)) {
        printUsage(argv[0]);
        return 1;
    }

    const auto value = makeValue(options.value_size);
    prewarm(options, value);

    std::atomic<int> next_request{0};
    std::atomic<int> success{0};
    std::atomic<int> failed{0};
    std::mutex latency_mutex;
    std::vector<double> latencies_ms;
    latencies_ms.reserve(static_cast<size_t>(options.requests));

    const auto begin = Clock::now();
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(options.clients));

    for (int c = 0; c < options.clients; ++c) {
        threads.emplace_back([&, c]() {
            lightkv::LightKVClient client(options.host, options.port);
            while (true) {
                const int request_id = next_request.fetch_add(1);
                if (request_id >= options.requests) {
                    break;
                }

                const auto op_begin = Clock::now();
                const int ratio_bucket = request_id % 10000;
                const bool do_read = ratio_bucket < static_cast<int>(options.read_ratio * 10000.0);
                const std::string key = do_read
                    ? "bench:key:" + std::to_string(request_id % std::max(1, options.requests))
                    : "bench:write:" + std::to_string(c) + ":" + std::to_string(request_id);
                const auto response = do_read ? client.get(key) : client.set(key, value);
                const auto op_end = Clock::now();
                const auto latency = std::chrono::duration<double, std::milli>(op_end - op_begin).count();

                if (isSuccess(response)) {
                    ++success;
                } else {
                    ++failed;
                }

                {
                    std::lock_guard<std::mutex> lock(latency_mutex);
                    latencies_ms.push_back(latency);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    const auto end = Clock::now();

    const auto elapsed_seconds = std::chrono::duration<double>(end - begin).count();
    double total_latency = 0.0;
    for (const auto latency : latencies_ms) {
        total_latency += latency;
    }

    const auto total = success.load() + failed.load();
    const auto avg_latency = latencies_ms.empty() ? 0.0 : total_latency / latencies_ms.size();
    auto p95_values = latencies_ms;
    auto p99_values = latencies_ms;
    const auto p95 = percentile(p95_values, 0.95);
    const auto p99 = percentile(p99_values, 0.99);
    const auto qps = elapsed_seconds > 0.0 ? static_cast<double>(total) / elapsed_seconds : 0.0;

    std::cout << "LightKV Bench Result\n";
    std::cout << "host: " << options.host << '\n';
    std::cout << "port: " << options.port << '\n';
    std::cout << "clients: " << options.clients << '\n';
    std::cout << "requests: " << options.requests << '\n';
    std::cout << "value_size: " << options.value_size << '\n';
    std::cout << "read_ratio: " << options.read_ratio << '\n';
    std::cout << "total_requests: " << total << '\n';
    std::cout << "success: " << success.load() << '\n';
    std::cout << "failed: " << failed.load() << '\n';
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "qps: " << qps << '\n';
    std::cout << "avg_latency_ms: " << avg_latency << '\n';
    std::cout << "p95_latency_ms: " << p95 << '\n';
    std::cout << "p99_latency_ms: " << p99 << '\n';

    return 0;
}
