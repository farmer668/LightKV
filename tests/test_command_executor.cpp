#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <chrono>
#include <string>
#include <thread>

int main() {
    lightkv::KVStore store;
    lightkv::Parser parser;
    lightkv::CommandExecutor executor(store);

    assert(executor.execute(parser.parseLine("PING")) == "+PONG\r\n");

    assert(executor.execute(parser.parseLine("SET name yifei")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("GET name")) == "$5\r\nyifei\r\n");

    assert(executor.execute(parser.parseLine("GET missing")) == "$-1\r\n");

    assert(executor.execute(parser.parseLine("EXISTS name")) == ":1\r\n");
    assert(executor.execute(parser.parseLine("EXISTS missing")) == ":0\r\n");

    assert(executor.execute(parser.parseLine("SIZE")) == ":1\r\n");

    assert(executor.execute(parser.parseLine("DEL name")) == ":1\r\n");
    assert(executor.execute(parser.parseLine("DEL name")) == ":0\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":0\r\n");

    assert(executor.execute(parser.parseLine("SET a 1")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("TTL a")) == ":-1\r\n");
    assert(executor.execute(parser.parseLine("EXPIRE a 10")) == ":1\r\n");
    const auto ttl_a = executor.execute(parser.parseLine("TTL a"));
    assert(ttl_a == ":10\r\n" || ttl_a == ":9\r\n");
    assert(executor.execute(parser.parseLine("GET a")) == "$1\r\n1\r\n");
    assert(executor.execute(parser.parseLine("EXPIRE missing 10")) == ":0\r\n");
    assert(executor.execute(parser.parseLine("TTL missing")) == ":-2\r\n");
    const auto invalid_expire = executor.execute(parser.parseLine("EXPIRE a abc"));
    assert(invalid_expire.rfind("-ERR", 0) == 0);

    assert(executor.execute(parser.parseLine("SET tmp 1")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("EXPIRE tmp 1")) == ":1\r\n");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    assert(executor.execute(parser.parseLine("GET tmp")) == "$-1\r\n");

    assert(executor.execute(parser.parseLine("SET token abc")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("EXPIRE token 10")) == ":1\r\n");
    const auto ttl_token = executor.execute(parser.parseLine("TTL token"));
    assert(ttl_token != ":-2\r\n");
    assert(executor.execute(parser.parseLine("GET token")) == "$3\r\nabc\r\n");

    assert(executor.execute(parser.parseLine("SET b 2")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":3\r\n");
    assert(executor.execute(parser.parseLine("CLEAR")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":0\r\n");

    const auto invalid_response = executor.execute(parser.parseLine("WHAT"));
    assert(invalid_response.rfind("-ERR", 0) == 0);

    assert(executor.execute(parser.parseLine("QUIT")) == "+BYE\r\n");

    return 0;
}
