cmake_minimum_required(VERSION 3.16)

project(LightKV
    VERSION 0.0.1
    DESCRIPTION "C++17 lightweight KV cache system"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
    message(STATUS "Building LightKV scaffold on Windows. Linux-only network modules will be added in later stages.")
elseif(UNIX)
    message(STATUS "Building LightKV for Linux/Unix target.")
endif()

set(LIGHTKV_CORE_SOURCES
    src/common/Status.cpp
    src/storage/KVStore.cpp
)

add_executable(lightkv_server
    src/main.cpp
    ${LIGHTKV_CORE_SOURCES}
)

target_include_directories(lightkv_server PRIVATE
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
