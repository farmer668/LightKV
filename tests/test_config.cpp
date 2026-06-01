#include "lightkv/common/Config.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    const std::string path = "data/test_config_stage7.conf";
    std::filesystem::create_directories("data");
    {
        std::ofstream out(path);
        out << "# comment\n";
        out << "\n";
        out << " host = 0.0.0.0 \n";
        out << "port=6380\n";
        out << "max_keys=42\n";
        out << "wal_enabled=yes\n";
        out << "bad_int=abc\n";
        out << "bad_bool=maybe\n";
    }

    lightkv::Config config;
    assert(config.loadFromFile(path));
    assert(config.getString("host", "127.0.0.1") == "0.0.0.0");
    assert(config.getInt("port", 1) == 6380);
    assert(config.getSizeT("max_keys", 1) == 42);
    assert(config.getBool("wal_enabled", false));
    assert(config.getString("missing", "default") == "default");
    assert(config.getInt("bad_int", 7) == 7);
    assert(config.getBool("bad_bool", true));

    config.set("feature", "false");
    assert(config.contains("feature"));
    assert(!config.getBool("feature", true));

    lightkv::Config missing;
    assert(!missing.loadFromFile("data/does_not_exist.conf"));
    assert(missing.getInt("port", 6379) == 6379);

    std::filesystem::remove(path);
    return 0;
}

