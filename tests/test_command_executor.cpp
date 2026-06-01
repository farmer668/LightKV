#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/common/Metrics.h"
#include "lightkv/persistence/Wal.h"
#include "lightkv/replication/ReplicationState.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <string>
#include <system_error>
#include <thread>

namespace {

const std::string kWalPath = "data/test_command_executor_stage8.wal";

void removeWal() {
    std::error_code ec;
    std::filesystem::remove(kWalPath, ec);
}

}  // namespace

int main() {
    lightkv::KVStore store;
    lightkv::Parser parser;
    lightkv::Metrics metrics;
    lightkv::CommandExecutor executor(store, nullptr, false, "", &metrics);

    assert(executor.execute(parser.parseLine("PING")) == "+PONG\r\n");

    assert(executor.execute(parser.parseLine("SET name yifei")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("GET name")) == "$5\r\nyifei\r\n");

    assert(executor.execute(parser.parseLine("GET missing")) == "$-1\r\n");

    assert(executor.execute(parser.parseLine("EXISTS name")) == ":1\r\n");
    assert(executor.execute(parser.parseLine("EXISTS missing")) == ":0\r\n");

    const auto info_response = executor.execute(parser.parseLine("INFO"));
    assert(info_response.find("keys:1") != std::string::npos);
    assert(info_response.find("max_keys:10000") != std::string::npos);
    assert(info_response.find("evicted_keys:0") != std::string::npos);
    assert(info_response.find("expired_keys:0") != std::string::npos);
    assert(info_response.find("wal_enabled:false") != std::string::npos);
    assert(info_response.find("wal_records:0") != std::string::npos);
    assert(info_response.find("total_commands:7") != std::string::npos);
    assert(info_response.find("ping_commands:1") != std::string::npos);
    assert(info_response.find("get_commands:2") != std::string::npos);
    assert(info_response.find("set_commands:1") != std::string::npos);
    assert(info_response.find("exists_commands:2") != std::string::npos);
    assert(info_response.find("info_commands:1") != std::string::npos);
    assert(info_response.find("hits:1") != std::string::npos);
    assert(info_response.find("misses:1") != std::string::npos);

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
    const auto get_token = executor.execute(parser.parseLine("GET token"));
    assert(get_token.find("abc") != std::string::npos);
    assert(get_token == "$3\r\nabc\r\n");

    assert(executor.execute(parser.parseLine("SET b 2")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":3\r\n");
    assert(executor.execute(parser.parseLine("CLEAR")) == "+OK\r\n");
    assert(executor.execute(parser.parseLine("SIZE")) == ":0\r\n");

    const auto invalid_response = executor.execute(parser.parseLine("WHAT"));
    assert(invalid_response.rfind("-ERR", 0) == 0);
    assert(executor.execute(parser.parseLine("REPLCONF listening-port 6380")) == "+OK\r\n");

    {
        lightkv::KVStore repl_store;
        lightkv::ReplicationState slave_state;
        slave_state.setRole(lightkv::ReplicationRole::Slave);
        lightkv::CommandExecutor slave_executor(repl_store, nullptr, false, "", nullptr, &slave_state);

        assert(slave_executor.execute(parser.parseLine("SET blocked 1")) == "-ERR slave is read-only\r\n");
        repl_store.set("readable", "yes");
        assert(slave_executor.execute(parser.parseLine("GET readable")) == "$3\r\nyes\r\n");
        assert(slave_executor.execute(parser.parseLine("SYNC 0")) == "-ERR only master can sync\r\n");
    }

    {
        removeWal();
        lightkv::KVStore repl_store;
        lightkv::Wal wal(kWalPath);
        assert(wal.open());
        lightkv::ReplicationState master_state;
        master_state.setRole(lightkv::ReplicationRole::Master);
        lightkv::CommandExecutor master_executor(repl_store, &wal, true, kWalPath, nullptr, &master_state);

        assert(master_executor.execute(parser.parseLine("SET rsync 1")) == "+OK\r\n");
        const auto sync_response = master_executor.execute(parser.parseLine("SYNC 0"));
        assert(sync_response.find("1|SET rsync 1") != std::string::npos);

        const auto repl_info = master_executor.execute(parser.parseLine("INFO"));
        assert(repl_info.find("role:master") != std::string::npos);
        assert(repl_info.find("replication_offset:1") != std::string::npos);
        wal.close();
        removeWal();
    }

    assert(executor.execute(parser.parseLine("QUIT")) == "+BYE\r\n");

    return 0;
}
