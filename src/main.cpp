#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cxxopts.hpp>

#include "../include/config.h"
#include "../include/openai.h"

// 检查是否是终端输入
bool is_terminal_input() {
    return isatty(fileno(stdin));
}

// 从stdin读取所有内容
std::string read_from_stdin() {
    std::stringstream buffer;
    buffer << std::cin.rdbuf();
    return buffer.str();
}

// 获取查询内容
std::string get_query(const cxxopts::ParseResult& args) {
    // 首先检查-q/--query选项
    if (args.count("query")) {
        return "Query: " + args["query"].as<std::string>();
    }
    
    // 然后检查位置参数
    if (args.count("positional")) {
        auto positional = args["positional"].as<std::vector<std::string>>();
        if (!positional.empty()) {
            std::string combined;
            for (size_t i = 0; i < positional.size(); ++i) {
                if (i > 0) combined += " ";
                combined += positional[i];
            }
            return "Query: " + combined;
        }
    }
    
    // 如果启用了记忆模式，允许空查询
    if (args.count("memory")) {
        return "";
    }
    
    // 如果只有管道输入，没有查询参数，将管道内容作为主要输入
    if (!is_terminal_input()) {
        return "";  // 将在get_input中处理
    }
    
    // 没有查询参数，返回空
    return "";
}

// 获取输入内容
std::string get_input() {
    if (!is_terminal_input()) {
        std::string input = read_from_stdin();
        if (!input.empty()) {
            return "Input: " + lc::openai::trim(input);
        }
    }
    return "";
}

int main(int argc, char** argv) {
    // 定义命令行参数
    cxxopts::Options options("lc", "Linux command-line AI assistant");
    
    options.add_options()
        ("q,query", "Specify the query for the AI", cxxopts::value<std::string>())
        ("m,memory", "Enable conversation memory")
        ("clear-memory", "Clear the conversation memory")
        ("show-memory", "Show the conversation memory")
        ("set", "Set a configuration value (key=value)", cxxopts::value<std::string>())
        ("show-config", "Show the current configuration")
        ("reset-config", "Reset the configuration to default values")
        ("model", "Override the default model for this request", cxxopts::value<std::string>())
        ("no-system-prompt", "Disable the system prompt for this request")
        ("debug", "Enable debug mode")
        ("h,help", "Print usage")
        ("positional", "Positional arguments", cxxopts::value<std::vector<std::string>>())
    ;
    
    options.parse_positional({"positional"});
    options.positional_help("<query>");
    
    // 解析命令行参数
    cxxopts::ParseResult args;
    try {
        args = options.parse(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }
    
    // 显示帮助
    if (args.count("help") || (argc == 1 && is_terminal_input())) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    
    bool debug = args.count("debug");
    
    // 加载配置
    std::optional<lc::Config> config_opt = lc::Config::load();
    if (!config_opt) {
        std::cerr << "Failed to load configuration" << std::endl;
        return 1;
    }
    lc::Config config = *config_opt;
    
    if (debug) {
        std::cerr << "Debug mode enabled" << std::endl;
        std::cerr << "Loaded configuration successfully" << std::endl;
    }
    
    // 处理配置相关命令
    if (args.count("show-config")) {
        config.show();
        return 0;
    }
    
    if (args.count("reset-config")) {
        if (lc::Config::reset_config()) {
            std::cout << "Configuration has been reset to default values." << std::endl;
        } else {
            std::cerr << "Failed to reset configuration." << std::endl;
            return 1;
        }
        return 0;
    }
    
    if (args.count("set")) {
        std::string set_arg = args["set"].as<std::string>();
        size_t pos = set_arg.find('=');
        if (pos == std::string::npos) {
            std::cerr << "Invalid set format. Use: --set key=value" << std::endl;
            return 1;
        }
        
        std::string key = set_arg.substr(0, pos);
        std::string value = set_arg.substr(pos + 1);
        
        if (config.set_value(key, value)) {
            std::cout << key << " set successfully." << std::endl;
        } else {
            std::cerr << "Failed to set " << key << "." << std::endl;
            return 1;
        }
        return 0;
    }
    
    // 处理记忆相关命令
    auto memory_path = lc::Config::memory_path();
    
    if (args.count("clear-memory")) {
        if (lc::openai::clear_messages(memory_path)) {
            std::cout << "Conversation memory has been cleared." << std::endl;
        } else {
            std::cerr << "Failed to clear conversation memory." << std::endl;
            return 1;
        }
        return 0;
    }
    
    if (args.count("show-memory")) {
        lc::openai::show_messages(memory_path);
        return 0;
    }
    
    // 获取查询和输入
    std::string query = get_query(args);
    std::string input = get_input();
    
    if (debug) {
        std::cerr << "Query: " << query << std::endl;
        std::cerr << "Input: " << input << std::endl;
    }
    
    // 如果没有输入和查询，并且不是记忆模式，显示帮助
    if (query.empty() && input.empty() && !args.count("memory")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    
    // 准备消息
    std::vector<lc::openai::Message> messages;
    
    // 添加系统提示
    if (!args.count("no-system-prompt") && config.use_system_prompt) {
        messages.push_back({"system", config.system_prompt});
    }
    
    // 加载历史消息
    if (args.count("memory")) {
        auto prev_messages = lc::openai::load_messages(memory_path);
        if (prev_messages) {
            messages.insert(messages.end(), prev_messages->begin(), prev_messages->end());
            
            if (debug) {
                std::cerr << "Loaded " << prev_messages->size() << " previous messages" << std::endl;
            }
        }
    }
    
    // 添加当前用户消息
    if (!query.empty() || !input.empty()) {
        std::string message_content = query;
        if (!input.empty()) {
            if (!message_content.empty()) {
                message_content += "\n\n";
            }
            message_content += input;
        }
        
        messages.push_back({"user", message_content});
    }
    
    if (debug) {
        std::cerr << "Total messages to send: " << messages.size() << std::endl;
    }
    
    // 获取模型覆盖（如果指定）
    std::string model_override;
    if (args.count("model")) {
        model_override = args["model"].as<std::string>();
        if (debug) {
            std::cerr << "Model override: " << model_override << std::endl;
        }
    }
    
    // 流式输出回调
    std::string accumulated_response;
    bool need_newline_at_end = false;
    auto stream_callback = [&accumulated_response, &need_newline_at_end, debug](const std::string& delta, bool is_done) {
        if (!is_done && !delta.empty()) {
            // 处理换行符，确保内容格式正确
            accumulated_response += delta;
            std::cout << delta << std::flush;
            
            // 如果消息不是以换行符结尾，那么最后需要添加一个换行符
            need_newline_at_end = (delta.back() != '\n');
        }
    };
    
    // 调用API进行聊天完成（流式）
    lc::openai::ChatCompletionResult result = lc::openai::chat_completion_stream(
        config,
        messages,
        stream_callback,
        model_override,
        debug
    );
    
    // 处理结果
    if (!result.success) {
        std::cerr << "Error: " << result.error_message << std::endl;
        return 1;
    }
    
    // 只有在需要时才添加最后的换行
    if (need_newline_at_end) {
        std::cout << std::endl;
    }
    
    // 保存对话历史
    if (args.count("memory") && result.success) {
        messages.push_back({"assistant", result.full_response});
        
        if (!lc::openai::save_messages(messages, memory_path, config.max_history)) {
            std::cerr << "Warning: Failed to save conversation history" << std::endl;
        } else if (debug) {
            std::cerr << "Saved conversation history" << std::endl;
        }
    }
    
    return 0;
}