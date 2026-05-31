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

add_executable(lightkv_server
    src/main.cpp
)

target_include_directories(lightkv_server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

enable_testing()

