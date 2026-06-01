#include "lightkv/storage/KVStore.h"
#include "lightkv/storage/LRUCache.h"

#include <cassert>
#include <chrono>
#include <thread>

int main() {
    {
        lightkv::LRUCache lru;
        lru.touch("a");
        lru.touch("b");
        lru.touch("c");
        assert(lru.size() == 3);
        assert(lru.contains("a"));
        assert(lru.evictCandidate().has_value());
        assert(lru.evictCandidate().value() == "a");

        lru.touch("a");
        assert(lru.evictCandidate().value() == "b");

        lru.remove("b");
        assert(!lru.contains("b"));
        assert(lru.size() == 2);

        lru.clear();
        assert(lru.size() == 0);
        assert(!lru.evictCandidate().has_value());
    }

    {
        lightkv::KVStore store(2);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.set("c", "3").ok());
        assert(!store.get("a").has_value());
        assert(store.get("b").value() == "2");
        assert(store.get("c").value() == "3");
        assert(store.evictedKeys() == 1);
        assert(store.size() == 2);
    }

    {
        lightkv::KVStore store(2);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.get("a").value() == "1");
        assert(store.set("c", "3").ok());
        assert(store.get("a").value() == "1");
        assert(!store.get("b").has_value());
        assert(store.get("c").value() == "3");
    }

    {
        lightkv::KVStore store(2);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.set("a", "10").ok());
        assert(store.set("c", "3").ok());
        assert(store.get("a").value() == "10");
        assert(!store.get("b").has_value());
        assert(store.get("c").value() == "3");
    }

    {
        lightkv::KVStore store(2);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.del("a"));
        assert(store.set("c", "3").ok());
        assert(store.get("b").value() == "2");
        assert(store.get("c").value() == "3");
        assert(store.evictedKeys() == 0);
    }

    {
        lightkv::KVStore store(2);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.expire("a", 1));
        std::this_thread::sleep_for(std::chrono::seconds(2));
        assert(!store.get("a").has_value());
        assert(store.set("c", "3").ok());
        assert(store.get("b").value() == "2");
        assert(store.get("c").value() == "3");
        assert(store.evictedKeys() == 0);
        assert(store.expiredKeys() == 1);
    }

    {
        lightkv::KVStore store(0);
        assert(store.set("a", "1").ok());
        assert(store.set("b", "2").ok());
        assert(store.set("c", "3").ok());
        assert(store.size() == 3);
        assert(store.evictedKeys() == 0);
    }

    return 0;
}

