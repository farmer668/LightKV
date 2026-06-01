#include "lightkv/common/Logger.h"

#include <cassert>

int main() {
    auto& logger = lightkv::Logger::instance();
    logger.setLevel(lightkv::LogLevel::DEBUG);
    logger.debug("debug message");
    logger.info("info message");
    logger.warn("warn message");
    logger.error("error message");

    logger.setLevel(lightkv::LogLevel::WARN);
    logger.info("filtered info message");
    logger.warn("visible warn message");

    assert(lightkv::parseLogLevel("DEBUG") == lightkv::LogLevel::DEBUG);
    assert(lightkv::parseLogLevel("info") == lightkv::LogLevel::INFO);
    assert(lightkv::parseLogLevel("WARN") == lightkv::LogLevel::WARN);
    assert(lightkv::parseLogLevel("ERROR") == lightkv::LogLevel::ERROR);
    assert(lightkv::parseLogLevel("bad", lightkv::LogLevel::ERROR) == lightkv::LogLevel::ERROR);
    assert(lightkv::logLevelToString(lightkv::LogLevel::INFO) == "INFO");

    return 0;
}

