#include "../include/config.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <stdexcept>

namespace lc {

// 默认系统提示
const char* DEFAULT_SYSTEM_PROMPT = R"(You are a professional Linux command-line assistant named lc. Your task is to answer users' questions about Linux commands, operations, and issues. Please follow these guidelines:

  1. Answer user questions directly, without using any Markdown formatting or text formatting (such as bold, italics, etc.).
  2. Keep answers concise and clear, suitable for display on a command-line interface.
  3. If the user provides command examples, carefully analyze and explain the role of each part.
  4. If errors or problems are encountered, provide possible causes and solutions.
  5. Use clear steps or numbered lists to explain complex processes.
  6. If you need to display code or commands, write them directly without using code block formatting.
  7. Avoid using emojis or other special characters that may display abnormally on the command line.
  8. If the user's question is unclear, politely request more information.
  9. Provide practical advice, including command best practices and security precautions.
  10. If the user requests an operation that may be risky, remind them of the potential consequences.
  11. Pay attention to the user's questions and requests, which are always in the Query. Please be sure to check them. The content in the Input is background or reference information.

  Remember, you must check the requirements in the received Query and the information in the Input, and your response will be displayed directly on the command-line interface, so keep the format simple and the content clear.)";

std::filesystem::path Config::lc_dir() {
    std::filesystem::path config_dir;
    
    // 获取用户配置目录
    #ifdef _WIN32
        const char* appdata = std::getenv("APPDATA");
        if (appdata) {
            config_dir = std::filesystem::path(appdata);
        }
    #else
        const char* xdg_config_home = std::getenv("XDG_CONFIG_HOME");
        if (xdg_config_home && *xdg_config_home) {
            config_dir = std::filesystem::path(xdg_config_home);
        } else {
            const char* home = std::getenv("HOME");
            if (home) {
                config_dir = std::filesystem::path(home) / ".config";
            }
        }
    #endif
    
    if (config_dir.empty()) {
        throw std::runtime_error("Failed to determine config directory");
    }
    
    std::filesystem::path lc_dir = config_dir / "lc";
    
    // 确保目录存在
    std::error_code ec;
    std::filesystem::create_directories(lc_dir, ec);
    if (ec) {
        throw std::runtime_error("Failed to create config directory: " + ec.message());
    }
    
    return lc_dir;
}

std::filesystem::path Config::config_path() {
    return lc_dir() / "config.yaml";
}

std::filesystem::path Config::memory_path() {
    return lc_dir() / "conversation_memory.json";
}

Config Config::default_config() {
    Config config;
    config.openai_api_key = "";
    config.openai_base_url = "https://api.openai.com/v1";
    config.default_model = "gpt-4o-mini";
    config.system_prompt = DEFAULT_SYSTEM_PROMPT;
    config.max_history = DEFAULT_MAX_HISTORY;
    config.use_system_prompt = true;
    return config;
}

std::optional<Config> Config::load() {
    auto path = config_path();
    
    // 检查文件是否存在
    if (!std::filesystem::exists(path)) {
        return default_config();
    }
    
    try {
        YAML::Node config = YAML::LoadFile(path.string());
        Config result;
        
        if (config["openai_api_key"]) {
            result.openai_api_key = config["openai_api_key"].as<std::string>();
        }
        
        if (config["openai_base_url"]) {
            result.openai_base_url = config["openai_base_url"].as<std::string>();
        } else {
            result.openai_base_url = "https://api.openai.com/v1";
        }
        
        if (config["default_model"]) {
            result.default_model = config["default_model"].as<std::string>();
        } else {
            result.default_model = "gpt-4o-mini";
        }
        
        if (config["system_prompt"]) {
            result.system_prompt = config["system_prompt"].as<std::string>();
        } else {
            result.system_prompt = DEFAULT_SYSTEM_PROMPT;
        }
        
        if (config["max_history"]) {
            result.max_history = config["max_history"].as<int>();
        } else {
            result.max_history = DEFAULT_MAX_HISTORY;
        }
        
        if (config["use_system_prompt"]) {
            result.use_system_prompt = config["use_system_prompt"].as<bool>();
        } else {
            result.use_system_prompt = true;
        }
        
        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        std::cerr << "Using default configuration." << std::endl;
        return default_config();
    }
}

bool Config::save() const {
    auto path = config_path();
    
    try {
        // 创建目录(如果不存在)
        std::filesystem::create_directories(path.parent_path());
        
        YAML::Node node;
        node["openai_api_key"] = openai_api_key;
        node["openai_base_url"] = openai_base_url;
        node["default_model"] = default_model;
        node["system_prompt"] = system_prompt;
        node["max_history"] = max_history;
        node["use_system_prompt"] = use_system_prompt;
        
        std::ofstream fout(path);
        if (!fout) {
            std::cerr << "Failed to open config file for writing: " << path << std::endl;
            return false;
        }
        
        fout << YAML::Dump(node);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

bool Config::set_value(const std::string& key, const std::string& value) {
    try {
        if (key == "openai_api_key") {
            openai_api_key = value;
        } else if (key == "openai_base_url") {
            openai_base_url = value;
        } else if (key == "default_model") {
            default_model = value;
        } else if (key == "system_prompt") {
            system_prompt = value;
        } else if (key == "max_history") {
            max_history = std::stoi(value);
            if (max_history < 0) {
                throw std::invalid_argument("max_history must be non-negative");
            }
        } else if (key == "use_system_prompt") {
            if (value == "true" || value == "1") {
                use_system_prompt = true;
            } else if (value == "false" || value == "0") {
                use_system_prompt = false;
            } else {
                throw std::invalid_argument("use_system_prompt must be true/false or 1/0");
            }
        } else {
            std::cerr << "Unknown config key: " << key << std::endl;
            return false;
        }
        
        return save();
    } catch (const std::exception& e) {
        std::cerr << "Error setting config value: " << e.what() << std::endl;
        return false;
    }
}

void Config::show() const {
    std::cout << "Current Configuration:" << std::endl;
    std::cout << "  openai_api_key: " << (openai_api_key.empty() ? "[NOT SET]" : "[HIDDEN]") << std::endl;
    std::cout << "  openai_base_url: " << openai_base_url << std::endl;
    std::cout << "  default_model: " << default_model << std::endl;
    std::cout << "  max_history: " << max_history << std::endl;
    std::cout << "  use_system_prompt: " << (use_system_prompt ? "true" : "false") << std::endl;
    std::cout << "  system_prompt: " << (system_prompt.length() > 50 ? system_prompt.substr(0, 47) + "..." : system_prompt) << std::endl;
}

bool Config::reset_config() {
    try {
        Config default_cfg = default_config();
        return default_cfg.save();
    } catch (const std::exception& e) {
        std::cerr << "Error resetting config: " << e.what() << std::endl;
        return false;
    }
}

} // namespace lc

// YAML 转换支持
namespace YAML {

Node convert<lc::Config>::encode(const lc::Config& config) {
    Node node;
    node["openai_api_key"] = config.openai_api_key;
    node["openai_base_url"] = config.openai_base_url;
    node["default_model"] = config.default_model;
    node["system_prompt"] = config.system_prompt;
    node["max_history"] = config.max_history;
    node["use_system_prompt"] = config.use_system_prompt;
    return node;
}

bool convert<lc::Config>::decode(const Node& node, lc::Config& config) {
    if (!node.IsMap()) {
        return false;
    }
    
    if (node["openai_api_key"]) {
        config.openai_api_key = node["openai_api_key"].as<std::string>();
    }
    
    if (node["openai_base_url"]) {
        config.openai_base_url = node["openai_base_url"].as<std::string>();
    }
    
    if (node["default_model"]) {
        config.default_model = node["default_model"].as<std::string>();
    }
    
    if (node["system_prompt"]) {
        config.system_prompt = node["system_prompt"].as<std::string>();
    }
    
    if (node["max_history"]) {
        config.max_history = node["max_history"].as<int>();
    }
    
    if (node["use_system_prompt"]) {
        config.use_system_prompt = node["use_system_prompt"].as<bool>();
    }
    
    return true;
}

} // namespace YAML