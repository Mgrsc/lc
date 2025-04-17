#ifndef LC_CONFIG_H
#define LC_CONFIG_H

#include <string>
#include <filesystem>
#include <optional>
#include <yaml-cpp/yaml.h>

namespace lc {

// 默认值常量
constexpr int DEFAULT_MAX_HISTORY = 10;
extern const char* DEFAULT_SYSTEM_PROMPT;

class Config {
public:
    // 配置项
    std::string openai_api_key;
    std::string openai_base_url;
    std::string default_model;
    std::string system_prompt;
    int max_history;
    bool use_system_prompt;

    // 加载配置
    static std::optional<Config> load();
    
    // 保存配置
    bool save() const;
    
    // 设置配置值
    bool set_value(const std::string& key, const std::string& value);
    
    // 获取配置路径
    static std::filesystem::path config_path();
    
    // 获取lc目录
    static std::filesystem::path lc_dir();
    
    // 获取记忆文件路径
    static std::filesystem::path memory_path();
    
    // 默认配置
    static Config default_config();
    
    // 显示当前配置
    void show() const;
    
    // 重置配置为默认值
    static bool reset_config();
};

} // namespace lc

// 为YAML-CPP提供转换支持
namespace YAML {
template<>
struct convert<lc::Config> {
    static Node encode(const lc::Config& config);
    static bool decode(const Node& node, lc::Config& config);
};
}

#endif // LC_CONFIG_H