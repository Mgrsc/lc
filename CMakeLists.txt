cmake_minimum_required(VERSION 3.14)
project(lc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")

include(FetchContent)

file(DOWNLOAD 
     "https://github.com/yhirose/cpp-httplib/raw/v0.20.0/httplib.h"
     "${CMAKE_CURRENT_BINARY_DIR}/include/httplib.h"
     SHOW_PROGRESS)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.1
)

FetchContent_MakeAvailable(nlohmann_json yaml-cpp cxxopts)

find_package(OpenSSL REQUIRED)

add_executable(lc
    src/main.cpp
    src/config.cpp
    src/openai.cpp
)

target_include_directories(lc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_definitions(lc PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

target_link_libraries(lc PRIVATE
    nlohmann_json::nlohmann_json
    yaml-cpp
    cxxopts::cxxopts
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(lc PRIVATE Threads::Threads)
endif()

install(TARGETS lc DESTINATION bin)