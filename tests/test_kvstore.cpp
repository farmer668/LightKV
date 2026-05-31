#include "lightkv/storage/KVStore.h"

#include <cassert>
#include <optional>
#include <string>

int main() {
    lightkv::KVStore store;

    assert(store.set("name", "LightKV").ok());
    const auto name = store.get("name");
    assert(name.has_value());
    assert(name.value() == "LightKV");

    assert(!store.get("missing").has_value());

    assert(store.exists("name"));
    assert(!store.exists("missing"));

    assert(store.size() == 1);
    assert(store.set("stage", "1").ok());
    assert(store.size() == 2);

    assert(store.del("name"));
    assert(!store.exists("name"));
    assert(store.size() == 1);

    assert(!store.del("missing"));

    store.clear();
    assert(store.size() == 0);
    assert(!store.exists("stage"));

    return 0;
}

