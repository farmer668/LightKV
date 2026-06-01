#include "lightkv/persistence/WalReplayer.h"
#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <vector>

int main() {
    {
        lightkv::KVStore store;
        lightkv::WalReplayer replayer(store);
        assert(replayer.replayCommand("SET a 1"));
        const auto value = store.get("a");
        assert(value.has_value());
        assert(value.value() == "1");
    }

    {
        lightkv::KVStore store;
        lightkv::WalReplayer replayer(store);
        assert(replayer.replayCommand("SET gone 1"));
        assert(replayer.replayCommand("DEL gone"));
        assert(!store.get("gone").has_value());
    }

    {
        lightkv::KVStore store;
        lightkv::WalReplayer replayer(store);
        assert(replayer.replayCommand("SET temp 1"));
        assert(replayer.replayCommand("EXPIRE temp 10"));
        assert(store.ttl("temp") != -2);
        assert(store.get("temp").has_value());
    }

    {
        lightkv::KVStore store;
        lightkv::WalReplayer replayer(store);
        const std::vector<lightkv::WalRecord> records = {
            {1, "SET a 1"},
            {2, "SET b 2"},
            {3, "DEL a"},
            {4, "BAD line"}};
        assert(replayer.replayRecords(records) == 3);
        assert(!store.get("a").has_value());
        const auto b = store.get("b");
        assert(b.has_value());
        assert(b.value() == "2");
    }

    return 0;
}
