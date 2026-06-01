#pragma once

#include <string>

namespace lightkv {

class LightKVClient {
public:
    LightKVClient(std::string host, int port);

    std::string sendCommand(const std::string& command);

    std::string set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    std::string del(const std::string& key);
    std::string expire(const std::string& key, int seconds);
    std::string ttl(const std::string& key);
    std::string info();

private:
    std::string host_;
    int port_;
};

}  // namespace lightkv
