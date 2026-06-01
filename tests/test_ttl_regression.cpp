#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <string>

int main() {
    lightkv::Parser parser;
    lightkv::KVStore store;
    lightkv::CommandExecutor executor(store);

    assert(executor.execute(parser.parseLine("SET token abc")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("EXPIRE token 10")) == ":1\r\n");

    const auto ttl_response = executor.execute(parser.parseLine("TTL token"));
    assert(ttl_response != ":-2\r\n");
    assert(!ttl_response.empty());
    assert(ttl_response.front() == ':');

    const auto get_response = executor.execute(parser.parseLine("GET token"));
    assert(get_response.find("abc") != std::string::npos);
    assert(get_response == "$3\r\nabc\r\n");

    return 0;
}

