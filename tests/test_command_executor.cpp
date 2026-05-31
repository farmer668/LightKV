#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <string>

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
    assert(executor.execute(parser.parseLine("SET b 2")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":2\r\n");
    assert(executor.execute(parser.parseLine("CLEAR")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":0\r\n");

    const auto invalid_response = executor.execute(parser.parseLine("WHAT"));
    assert(invalid_response.rfind("-ERR", 0) == 0);

    assert(executor.execute(parser.parseLine("QUIT")) == "+BYE\r\n");

    return 0;
}

