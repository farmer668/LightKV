cmake_minimum_required(VERSION 3.16)

project(LightKV
    VERSION 0.0.1
    DESCRIPTION "C++17 lightweight KV cache system"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

if(WIN32)
    message(STATUS "Building LightKV scaffold on Windows. Linux-only network modules will be added in later stages.")
elseif(UNIX AND NOT APPLE)
    message(STATUS "Building LightKV for Linux/Unix target.")
endif()

set(LIGHTKV_CORE_SOURCES
    src/common/Config.cpp
    src/common/Logger.cpp
    src/common/Metrics.cpp
    src/common/Status.cpp
    src/storage/LRUCache.cpp
    src/storage/KVStore.cpp
)

set(LIGHTKV_PROTOCOL_SOURCES
    src/protocol/Parser.cpp
    src/protocol/Response.cpp
    src/protocol/CommandExecutor.cpp
    src/persistence/Wal.cpp
)

set(LIGHTKV_NET_COMMON_SOURCES
    src/net/Buffer.cpp
)

set(LIGHTKV_SERVER_SOURCES
    src/main.cpp
    ${LIGHTKV_CORE_SOURCES}
    ${LIGHTKV_PROTOCOL_SOURCES}
)

if(UNIX AND NOT APPLE)
    list(APPEND LIGHTKV_SERVER_SOURCES
        ${LIGHTKV_NET_COMMON_SOURCES}
        src/net/TcpConnection.cpp
        src/net/TcpServer.cpp
    )
endif()

add_executable(lightkv_server
    ${LIGHTKV_SERVER_SOURCES}
)

target_include_directories(lightkv_server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

if(UNIX AND NOT APPLE)
    target_compile_definitions(lightkv_server PRIVATE LIGHTKV_ENABLE_TCP_SERVER)
    target_link_libraries(lightkv_server PRIVATE Threads::Threads)
endif()

add_executable(lightkv_cli
    tools/lightkv_cli.cpp
    ${LIGHTKV_CORE_SOURCES}
    ${LIGHTKV_PROTOCOL_SOURCES}
)

target_include_directories(lightkv_cli PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

enable_testing()

add_executable(test_kvstore
    tests/test_kvstore.cpp
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_kvstore PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_kvstore COMMAND test_kvstore)

add_executable(test_parser
    tests/test_parser.cpp
    ${LIGHTKV_PROTOCOL_SOURCES}
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_parser PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_parser COMMAND test_parser)

add_executable(test_command_executor
    tests/test_command_executor.cpp
    ${LIGHTKV_PROTOCOL_SOURCES}
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_command_executor PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(test_command_executor PRIVATE Threads::Threads)

add_test(NAME test_command_executor COMMAND test_command_executor)

add_executable(test_buffer
    tests/test_buffer.cpp
    ${LIGHTKV_NET_COMMON_SOURCES}
)

target_include_directories(test_buffer PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_buffer COMMAND test_buffer)

add_executable(test_ttl
    tests/test_ttl.cpp
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_ttl PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(test_ttl PRIVATE Threads::Threads)

add_test(NAME test_ttl COMMAND test_ttl)

add_executable(test_ttl_regression
    tests/test_ttl_regression.cpp
    ${LIGHTKV_PROTOCOL_SOURCES}
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_ttl_regression PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_ttl_regression COMMAND test_ttl_regression)

add_executable(test_lru
    tests/test_lru.cpp
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_lru PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(test_lru PRIVATE Threads::Threads)

add_test(NAME test_lru COMMAND test_lru)

add_executable(test_wal
    tests/test_wal.cpp
    ${LIGHTKV_PROTOCOL_SOURCES}
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(test_wal PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_wal COMMAND test_wal)

add_executable(test_config
    tests/test_config.cpp
    src/common/Config.cpp
    src/common/Logger.cpp
)

target_include_directories(test_config PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_config COMMAND test_config)

add_executable(test_logger
    tests/test_logger.cpp
    src/common/Logger.cpp
)

target_include_directories(test_logger PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_logger COMMAND test_logger)

add_executable(test_metrics
    tests/test_metrics.cpp
    src/common/Metrics.cpp
)

target_include_directories(test_metrics PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

add_test(NAME test_metrics COMMAND test_metrics)
