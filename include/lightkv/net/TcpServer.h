#pragma once

#include "lightkv/net/TcpConnection.h"
#include "lightkv/protocol/CommandExecutor.h"
#include "lightkv/protocol/Parser.h"
#include "lightkv/storage/KVStore.h"

#include <atomic>
#include <string>
#include <thread>
#include <unordered_map>

namespace lightkv {

class TcpServer {
public:
    TcpServer(std::string host, int port);
    ~TcpServer();

    bool start();
    void run();

private:
    bool setupListenSocket();
    void startExpireWorker();
    void stopExpireWorker();
    bool setNonBlocking(int fd);
    void handleAccept();
    void handleClientRead(int client_fd);
    bool sendResponse(int client_fd, const std::string& response);
    void closeConnection(int client_fd);

    std::string host_;
    int port_;
    int listen_fd_ = -1;
    int epoll_fd_ = -1;
    KVStore store_;
    Parser parser_;
    CommandExecutor executor_;
    std::unordered_map<int, TcpConnection> connections_;
    std::atomic<bool> running_{false};
    std::thread expire_thread_;
};

}  // namespace lightkv
