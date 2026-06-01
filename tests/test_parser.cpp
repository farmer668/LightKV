#include "lightkv/protocol/Parser.h"

#include <cassert>
#include <string>

namespace {

void assertType(const lightkv::Command& command, lightkv::CommandType type) {
    assert(command.type == type);
    assert(command.error.empty());
}

}  // namespace

int main() {
    lightkv::Parser parser;

    auto command = parser.parseLine("PING");
    assertType(command, lightkv::CommandType::Ping);
    assert(command.args.empty());
    assert(command.raw == "PING");

    command = parser.parseLine("SET name yifei");
    assertType(command, lightkv::CommandType::Set);
    assert(command.args.size() == 2);
    assert(command.args[0] == "name");
    assert(command.args[1] == "yifei");

    command = parser.parseLine("GET name");
    assertType(command, lightkv::CommandType::Get);
    assert(command.args.size() == 1);
    assert(command.args[0] == "name");

    command = parser.parseLine("DEL name");
    assertType(command, lightkv::CommandType::Del);
    assert(command.args.size() == 1);
    assert(command.args[0] == "name");

    command = parser.parseLine("EXISTS name");
    assertType(command, lightkv::CommandType::Exists);
    assert(command.args.size() == 1);
    assert(command.args[0] == "name");

    command = parser.parseLine("EXPIRE name 10");
    assertType(command, lightkv::CommandType::Expire);
    assert(command.args.size() == 2);
    assert(command.args[0] == "name");
    assert(command.args[1] == "10");

    command = parser.parseLine("INFO");
    assertType(command, lightkv::CommandType::Info);
    assert(command.args.empty());

    command = parser.parseLine("SYNC 7");
    assertType(command, lightkv::CommandType::Sync);
    assert(command.args.size() == 1);
    assert(command.args[0] == "7");

    command = parser.parseLine("REPLCONF listening-port 6380");
    assertType(command, lightkv::CommandType::ReplConf);
    assert(command.args.size() == 2);
    assert(command.args[0] == "listening-port");
    assert(command.args[1] == "6380");

    command = parser.parseLine("SIZE");
    assertType(command, lightkv::CommandType::Size);
    assert(command.args.empty());

    command = parser.parseLine("TTL name");
    assertType(command, lightkv::CommandType::Ttl);
    assert(command.args.size() == 1);
    assert(command.args[0] == "name");

    command = parser.parseLine("CLEAR");
    assertType(command, lightkv::CommandType::Clear);
    assert(command.args.empty());

    command = parser.parseLine("QUIT");
    assertType(command, lightkv::CommandType::Quit);
    assert(command.args.empty());

    command = parser.parseLine("UNKNOWN");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    command = parser.parseLine("SET only_key");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    command = parser.parseLine("EXPIRE name");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    command = parser.parseLine("TTL");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    command = parser.parseLine("SYNC");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    command = parser.parseLine("set mixed Case");
    assertType(command, lightkv::CommandType::Set);
    assert(command.args.size() == 2);
    assert(command.args[0] == "mixed");
    assert(command.args[1] == "Case");

    command = parser.parseLine("expire name 10");
    assertType(command, lightkv::CommandType::Expire);
    assert(command.args.size() == 2);
    assert(command.args[0] == "name");
    assert(command.args[1] == "10");

    command = parser.parseLine("info");
    assertType(command, lightkv::CommandType::Info);
    assert(command.args.empty());

    command = parser.parseLine("   ");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(command.error == "empty command");

    command = parser.parseLine("SET msg hello world");
    assert(command.type == lightkv::CommandType::Invalid);
    assert(!command.error.empty());

    return 0;
}
