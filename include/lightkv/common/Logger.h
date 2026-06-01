#pragma once

#include <mutex>
#include <string>

namespace lightkv {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& instance();

    void setLevel(LogLevel level);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger() = default;

    void log(LogLevel level, const std::string& msg);

    LogLevel level_ = LogLevel::INFO;
    std::mutex mutex_;
};

LogLevel parseLogLevel(const std::string& value, LogLevel default_level = LogLevel::INFO);
std::string logLevelToString(LogLevel level);

}  // namespace lightkv

