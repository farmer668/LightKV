#include "lightkv/common/Logger.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace lightkv {

namespace {

std::string toUpper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string nowText() {
    const auto now = std::chrono::system_clock::now();
    const auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

}  // namespace

Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

void Logger::debug(const std::string& msg) {
    log(LogLevel::DEBUG, msg);
}

void Logger::info(const std::string& msg) {
    log(LogLevel::INFO, msg);
}

void Logger::warn(const std::string& msg) {
    log(LogLevel::WARN, msg);
}

void Logger::error(const std::string& msg) {
    log(LogLevel::ERROR, msg);
}

void Logger::log(LogLevel level, const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (static_cast<int>(level) < static_cast<int>(level_)) {
        return;
    }

    std::cout << "[" << nowText() << "] [" << logLevelToString(level) << "] " << msg << '\n';
}

LogLevel parseLogLevel(const std::string& value, LogLevel default_level) {
    const auto upper = toUpper(value);
    if (upper == "DEBUG") {
        return LogLevel::DEBUG;
    }
    if (upper == "INFO") {
        return LogLevel::INFO;
    }
    if (upper == "WARN" || upper == "WARNING") {
        return LogLevel::WARN;
    }
    if (upper == "ERROR") {
        return LogLevel::ERROR;
    }
    return default_level;
}

std::string logLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARN:
            return "WARN";
        case LogLevel::ERROR:
            return "ERROR";
    }

    return "INFO";
}

}  // namespace lightkv
