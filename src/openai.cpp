
#include "../include/openai.h"
#include <httplib.h>
#include <regex>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace lc {
namespace openai {

// 字符串修剪函数
std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

// Message 转为 JSON
nlohmann::json Message::to_json() const {
    nlohmann::json j;
    j["role"] = role;
    j["content"] = content;
    return j;
}

// 从JSON创建Message
Message Message::from_json(const nlohmann::json& j) {
    Message msg;
    msg.role = j["role"].get<std::string>();
    msg.content = j["content"].get<std::string>();
    return msg;
}

// 规范化API URL - 修复：保留尾部斜杠，避免308重定向问题
std::string normalize_api_url(const std::string& base_url) {
    std::string url = base_url;
    
    // 确保URL以斜杠结尾，避免308重定向问题
    if (!url.empty() && url.back() != '/') {
        url += '/';
    }
    
    return url;
}

// 创建HTTP客户端
std::unique_ptr<httplib::Client> create_http_client(const std::string& host, bool use_https, bool debug) {
    std::unique_ptr<httplib::Client> client;
    
    if (use_https) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        client = std::make_unique<httplib::Client>(host);
        client->set_ca_cert_path("/etc/ssl/certs");
        // 禁用SSL证书验证，避免SSL证书问题
        client->enable_server_certificate_verification(false);
        if (debug) {
            std::cerr << "SSL certificate verification disabled for debugging purposes" << std::endl;
        }
#else
        if (debug) {
            std::cerr << "HTTPS support not available. Rebuild with OpenSSL support." << std::endl;
        }
        return nullptr;
#endif
    } else {
        client = std::make_unique<httplib::Client>(host);
    }
    
    client->set_connection_timeout(30);
    client->set_read_timeout(120);
    client->set_write_timeout(30);
    client->set_follow_location(true);
    
    return client;
}

// 解析API URL
bool parse_api_url(const std::string& url_base, std::string& host, std::string& path_prefix, bool& use_https, bool debug) {
    std::regex url_regex(R"(^(https?)://([^/]+)(/.*)?$)");
    std::smatch url_match;
    
    if (std::regex_match(url_base, url_match, url_regex)) {
        std::string protocol = url_match[1].str();
        host = url_match[2].str();
        path_prefix = url_match[3].matched ? url_match[3].str() : "";
        use_https = (protocol == "https");
        
        if (debug) {
            std::cerr << "URL parsed - Protocol: " << protocol 
                      << ", Host: " << host 
                      << ", Path prefix: " << path_prefix << std::endl;
        }
        
        return true;
    }
    
    return false;
}

// 执行非流式聊天完成请求
ChatCompletionResult chat_completion(
    const Config& config, 
    const std::vector<Message>& messages, 
    const std::string& model_override,
    bool debug
) {
    ChatCompletionResult result;
    result.success = false;
    
    // 准备请求URL
    std::string url_base = normalize_api_url(config.openai_base_url);
    std::string host;
    std::string path_prefix;
    bool use_https;
    
    if (!parse_api_url(url_base, host, path_prefix, use_https, debug)) {
        result.error_message = "Invalid base URL: " + url_base;
        return result;
    }
    
    // 构建完整路径，移除末尾的斜杠，避免路径中有双斜杠
    if (!path_prefix.empty() && path_prefix.back() == '/') {
        path_prefix.pop_back();
    }
    std::string path = path_prefix + "/chat/completions";
    
    // 准备请求体
    nlohmann::json request_body;
    request_body["model"] = model_override.empty() ? config.default_model : model_override;
    
    nlohmann::json messages_json = nlohmann::json::array();
    for (const auto& msg : messages) {
        messages_json.push_back(msg.to_json());
    }
    
    request_body["messages"] = messages_json;
    
    std::string request_body_str = request_body.dump();
    
    if (debug) {
        std::cerr << "Request URL: " << (use_https ? "https://" : "http://") << host << path << std::endl;
        std::cerr << "Request body: " << request_body_str << std::endl;
    }
    
    // 创建HTTP客户端
    auto client = create_http_client(host, use_https, debug);
    if (!client) {
        result.error_message = "Failed to create HTTP client";
        return result;
    }
    
    // 设置请求头
    httplib::Headers headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + config.openai_api_key}
    };
    
    // 发送请求
    auto http_result = client->Post(path, headers, request_body_str, "application/json");
    
    if (!http_result) {
        result.error_message = "HTTP request failed: " + 
                             httplib::to_string(http_result.error());
        return result;
    }
    
    if (debug) {
        std::cerr << "Response status: " << http_result->status << std::endl;
        std::cerr << "Response body: " << http_result->body << std::endl;
    }
    
    if (http_result->status != 200) {
        result.error_message = "API request failed with status " + 
                             std::to_string(http_result->status) + ": " + 
                             http_result->body;
        return result;
    }
    
    try {
        nlohmann::json response_json = nlohmann::json::parse(http_result->body);
        
        if (!response_json.contains("choices") || 
            response_json["choices"].empty() ||
            !response_json["choices"][0].contains("message") ||
            !response_json["choices"][0]["message"].contains("content")) {
            
            result.error_message = "Invalid API response format";
            return result;
        }
        
        std::string content = response_json["choices"][0]["message"]["content"].get<std::string>();
        
        result.full_response = trim(content);
        result.success = true;
        
        return result;
    } catch (const std::exception& e) {
        result.error_message = std::string("Failed to parse API response: ") + e.what();
        return result;
    }
}

// 执行流式聊天完成请求 - 由于cpp-httplib的限制，我们需要简化实现方式
ChatCompletionResult chat_completion_stream(
    const Config& config, 
    const std::vector<Message>& messages, 
    StreamCallback callback,
    const std::string& model_override,
    bool debug
) {
    ChatCompletionResult result;
    result.success = false;
    result.full_response = "";
    
    // 准备请求URL
    std::string url_base = normalize_api_url(config.openai_base_url);
    std::string host;
    std::string path_prefix;
    bool use_https;
    
    if (!parse_api_url(url_base, host, path_prefix, use_https, debug)) {
        result.error_message = "Invalid base URL: " + url_base;
        callback("", true); // 通知完成
        return result;
    }
    
    // 构建完整路径，移除末尾的斜杠，避免路径中有双斜杠
    if (!path_prefix.empty() && path_prefix.back() == '/') {
        path_prefix.pop_back();
    }
    std::string path = path_prefix + "/chat/completions";
    
    // 准备请求体
    nlohmann::json request_body;
    request_body["model"] = model_override.empty() ? config.default_model : model_override;
    request_body["stream"] = true;
    
    nlohmann::json messages_json = nlohmann::json::array();
    for (const auto& msg : messages) {
        messages_json.push_back(msg.to_json());
    }
    
    request_body["messages"] = messages_json;
    
    std::string request_body_str = request_body.dump();
    
    if (debug) {
        std::cerr << "Request URL: " << (use_https ? "https://" : "http://") << host << path << std::endl;
        std::cerr << "Request body: " << request_body_str << std::endl;
    }
    
    // 创建HTTP客户端
    auto client = create_http_client(host, use_https, debug);
    if (!client) {
        result.error_message = "Failed to create HTTP client";
        callback("", true); // 通知完成
        return result;
    }
    
    // 设置请求头
    httplib::Headers headers = {
        {"Content-Type", "application/json"},
        {"Authorization", "Bearer " + config.openai_api_key},
        {"Accept", "text/event-stream"}
    };
    
    // 发送请求 - 由于cpp-httplib的限制，我们不使用回调方式，而是获取完整响应后手动解析
    auto http_result = client->Post(path, headers, request_body_str, "application/json");
    
    if (!http_result) {
        result.error_message = "HTTP request failed: " + 
                             httplib::to_string(http_result.error());
        callback("", true);  // 通知完成
        return result;
    }
    
    if (http_result->status == 308) {  // 永久重定向
        // 解析重定向位置
        if (http_result->has_header("Location")) {
            std::string new_location = http_result->get_header_value("Location");
            
            if (debug) {
                std::cerr << "Got 308 redirect to: " << new_location << std::endl;
            }
            
            // 尝试解析新位置
            std::string redirect_host;
            std::string redirect_path_prefix;
            bool redirect_use_https;
            
            if (parse_api_url(new_location, redirect_host, redirect_path_prefix, redirect_use_https, debug)) {
                if (debug) {
                    std::cerr << "Redirect parsed - New host: " << redirect_host << ", New path: " << redirect_path_prefix << std::endl;
                }
            }
        }
        
        result.error_message = "API request failed with status 308 (Permanent Redirect). "
                             "Please check your openai_base_url setting. "
                             "Try adding a trailing slash: " + url_base;
        callback("", true);  // 通知完成
        return result;
    }
    
    if (http_result->status != 200) {
        result.error_message = "API request failed with status " + 
                             std::to_string(http_result->status) + ": " + 
                             http_result->body;
        callback("", true);  // 通知完成
        return result;
    }
    
    // 手动解析SSE响应
    std::string accumulated_response;
    std::istringstream response_stream(http_result->body);
    std::string line;
    
    while (std::getline(response_stream, line)) {
        line = trim(line);
        
        // 跳过空行和注释
        if (line.empty() || line.find(":") == 0) {
            continue;
        }
        
        // 检查是否是data前缀
        if (line.find("data: ") == 0) {
            std::string data = line.substr(6);
            
            // 处理流结束标记
            if (data == "[DONE]") {
                callback("", true);  // 通知流结束
                break;
            }
            
            try {
                nlohmann::json data_json = nlohmann::json::parse(data);
                
                // 提取内容增量
                if (data_json.contains("choices") && 
                    !data_json["choices"].empty() && 
                    data_json["choices"][0].contains("delta") && 
                    data_json["choices"][0]["delta"].contains("content")) {
                    
                    std::string content_delta = data_json["choices"][0]["delta"]["content"].get<std::string>();
                    
                    if (!content_delta.empty()) {
                        accumulated_response += content_delta;
                        callback(content_delta, false);
                    }
                }
            } catch (const std::exception& e) {
                if (debug) {
                    std::cerr << "Error parsing data: " << e.what() << std::endl;
                }
            }
        }
    }
    
    result.success = true;
    result.full_response = trim(accumulated_response);
    
    return result;
}

// 加载消息历史
std::optional<std::vector<Message>> load_messages(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }
    
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }
        
        nlohmann::json json_data;
        file >> json_data;
        
        if (!json_data.is_array()) {
            return std::nullopt;
        }
        
        std::vector<Message> messages;
        for (const auto& msg_json : json_data) {
            messages.push_back(Message::from_json(msg_json));
        }
        
        return messages;
    } catch (const std::exception& e) {
        std::cerr << "Error loading messages: " << e.what() << std::endl;
        return std::nullopt;
    }
}

// 保存消息历史
bool save_messages(
    const std::vector<Message>& messages, 
    const std::filesystem::path& path, 
    int max_history
) {
    try {
        // 创建父目录(如果不存在)
        std::filesystem::create_directories(path.parent_path());
        
        // 筛选消息
        std::vector<Message> filtered_messages;
        
        // 过滤系统消息，保留最近的max_history条用户/助手消息
        for (auto it = messages.rbegin(); it != messages.rend() && filtered_messages.size() < (size_t)max_history * 2; ++it) {
            if (it->role != "system") {
                filtered_messages.push_back(*it);
            }
        }
        
        // 恢复正确顺序
        std::reverse(filtered_messages.begin(), filtered_messages.end());
        
        // 序列化为JSON
        nlohmann::json json_array = nlohmann::json::array();
        for (const auto& msg : filtered_messages) {
            json_array.push_back(msg.to_json());
        }
        
        // 写入文件
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }
        
        file << json_array.dump(2);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving messages: " << e.what() << std::endl;
        return false;
    }
}

// 清除消息历史
bool clear_messages(const std::filesystem::path& path) {
    try {
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
            return true;
        }
        return true; // 文件不存在也算成功
    } catch (const std::exception& e) {
        std::cerr << "Error clearing messages: " << e.what() << std::endl;
        return false;
    }
}

// 显示消息历史
void show_messages(const std::filesystem::path& path) {
    auto messages_opt = load_messages(path);
    
    if (!messages_opt || messages_opt->empty()) {
        std::cout << "No conversation history found." << std::endl;
        return;
    }
    
    const auto& messages = *messages_opt;
    
    std::cout << "Conversation History:" << std::endl;
    std::cout << "-----------------------------------------" << std::endl;
    
    int msg_count = 0;
    for (const auto& msg : messages) {
        std::string role_display = msg.role;
        if (role_display == "user") {
            role_display = "User";
        } else if (role_display == "assistant") {
            role_display = "Assistant";
        } else if (role_display == "system") {
            role_display = "System";
        }
        
        std::cout << "[" << role_display << "]:" << std::endl;
        
        // 截断显示过长的消息
        constexpr size_t max_display_length = 500;
        std::string content = msg.content;
        bool truncated = false;
        
        if (content.length() > max_display_length) {
            content = content.substr(0, max_display_length);
            truncated = true;
        }
        
        std::cout << content;
        if (truncated) {
            std::cout << "... [truncated]";
        }
        
        std::cout << std::endl << std::endl;
        msg_count++;
    }
    
    std::cout << "-----------------------------------------" << std::endl;
    std::cout << "Total messages: " << msg_count << std::endl;
}

} // namespace openai
} // namespace lc

// JSON 序列化支持
namespace nlohmann {

void adl_serializer<lc::openai::Message>::to_json(json& j, const lc::openai::Message& message) {
    j = message.to_json();
}

void adl_serializer<lc::openai::Message>::from_json(const json& j, lc::openai::Message& message) {
    message = lc::openai::Message::from_json(j);
}

} // namespace nlohmann
