cmake_minimum_required(VERSION 3.14)
project(lc VERSION 0.1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 获取外部依赖
include(FetchContent)

# 不使用 FetchContent 获取 httplib，而是直接下载单头文件版本
file(DOWNLOAD 
     "https://github.com/yhirose/cpp-httplib/raw/v0.20.0/httplib.h"
     "${CMAKE_CURRENT_BINARY_DIR}/include/httplib.h"
     SHOW_PROGRESS)

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

# 使所有依赖可用
FetchContent_MakeAvailable(nlohmann_json yaml-cpp cxxopts)

# 添加可执行文件
add_executable(lc
    src/main.cpp
    src/config.cpp
    src/openai.cpp
)

# 设置包含路径
target_include_directories(lc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include  # 添加下载的 httplib.h 所在目录
)

# 定义 CPPHTTPLIB_OPENSSL_SUPPORT 以启用 SSL 支持
target_compile_definitions(lc PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)

# 链接依赖库
target_link_libraries(lc PRIVATE
    nlohmann_json::nlohmann_json
    yaml-cpp
    cxxopts::cxxopts
    OpenSSL::SSL
    OpenSSL::Crypto
)

# 查找 OpenSSL
find_package(OpenSSL REQUIRED)

# 根据平台添加必要的链接
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(lc PRIVATE Threads::Threads)
endif()

# 安装目标
install(TARGETS lc DESTINATION bin)
