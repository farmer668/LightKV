#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <chrono>
#include <thread>

int main() {
    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.expire("a", 10));
        assert(store.ttl("a") >= 0);
        const auto value = store.get("a");
        assert(value.has_value());
        assert(value.value() == "1");
    }

    {
        lightkv::KVStore store;
        assert(!store.expire("missing", 10));
    }

    {
        lightkv::KVStore store;
        assert(store.ttl("missing") == -2);
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.ttl("a") == -1);
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.expire("a", 1));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        assert(!store.get("a").has_value());
        assert(!store.exists("a"));
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.expire("a", 0));
        assert(!store.get("a").has_value());
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.expire("a", 1));
        assert(store.set("a", "2").ok());
        assert(store.ttl("a") == -1);
        std::this_thread::sleep_for(std::chrono::seconds(2));
        const auto value = store.get("a");
        assert(value.has_value());
        assert(value.value() == "2");
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.set("c", "3").ok());
        assert(store.expire("a", 1));
        assert(store.expire("b", 1));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        assert(store.cleanupExpired() == 2);
        assert(store.size() == 1);
        assert(store.exists("c"));
    }

    return 0;
}

