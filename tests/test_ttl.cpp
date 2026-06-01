#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <chrono>
#include <thread>

namespace {

void test_get_before_expire_after_expire_set() {
    lightkv::KVStore store;
    assert(store.set("token", "abc").ok());
    assert(store.expire("token", 10));

    const auto remaining = store.ttl("token");
    assert(remaining >= 0);

    const auto value = store.get("token");
    assert(value.has_value());
    assert(value.value() == "abc");
}

}  // namespace

int main() {
    test_get_before_expire_after_expire_set();

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
        assert(store.set("token", "abc").ok());
        assert(store.expire("token", 1));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        assert(!store.get("token").has_value());
        assert(store.ttl("token") == -2);
    }

    {
        lightkv::KVStore store;
        assert(store.set("a", "1").ok());
        assert(store.expire("a", 10));
        const long long remaining = store.ttl("a");
        assert(remaining >= 0);
        const auto value = store.get("a");
        assert(value.has_value());
        assert(value.value() == "1");
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

    {
        lightkv::KVStore store;
        assert(store.set("live", "1").ok());
        assert(store.expire("live", 10));
        assert(store.cleanupExpired() == 0);
        const auto value = store.get("live");
        assert(value.has_value());
        assert(value.value() == "1");
    }

    return 0;
}
