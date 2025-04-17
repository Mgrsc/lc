cmake_minimum_required(VERSION 3.14)
project(lc VERSION 0.1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 获取外部依赖
include(FetchContent)

# 获取httplib - HTTP客户端，禁用压缩相关功能
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.20.0
)

# 禁用 httplib 的压缩功能，以避免 zstd 依赖
set(HTTPLIB_REQUIRE_ZLIB OFF CACHE BOOL "Require zlib for httplib" FORCE)
set(HTTPLIB_COMPILE ON CACHE BOOL "Build cpp-httplib" FORCE)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF CACHE BOOL "Use zlib if available" FORCE)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF CACHE BOOL "Use brotli if available" FORCE)
set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE ON CACHE BOOL "Use OpenSSL if available" FORCE)

# 获取nlohmann_json - JSON解析
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

# 获取yaml-cpp - YAML解析
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
)

# 获取cxxopts - 命令行参数解析
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG v3.2.1
)

# 移除 find_package(Zstd REQUIRED) 这一行

# 使所有依赖可用
FetchContent_MakeAvailable(httplib nlohmann_json yaml-cpp cxxopts)

# 添加可执行文件
add_executable(lc
    src/main.cpp
    src/config.cpp
    src/openai.cpp
)

# 设置包含路径
target_include_directories(lc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${httplib_SOURCE_DIR}/
)

# 链接依赖库
target_link_libraries(lc PRIVATE
    httplib::httplib
    nlohmann_json::nlohmann_json
    yaml-cpp
    cxxopts::cxxopts
)

# 根据平台添加必要的链接
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(lc PRIVATE Threads::Threads)
endif()

# 安装目标
install(TARGETS lc DESTINATION bin)
