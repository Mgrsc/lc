#ifndef LC_OPENAI_H
#define LC_OPENAI_H

#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <nlohmann/json.hpp>

#include "config.h"

namespace lc {
namespace openai {

// 消息结构体
struct Message {
    std::string role;
    std::string content;
    
    // JSON序列化支持
    nlohmann::json to_json() const;
    static Message from_json(const nlohmann::json& j);
};

// 流式回调函数类型
using StreamCallback = std::function<void(const std::string& delta, bool is_done)>;

// 聊天完成结果
struct ChatCompletionResult {
    bool success;
    std::string full_response;
    std::string error_message;
};

// 请求聊天完成（非流式）
ChatCompletionResult chat_completion(
    const Config& config, 
    const std::vector<Message>& messages, 
    const std::string& model_override = "",
    bool debug = false
);

// 请求聊天完成（流式）
ChatCompletionResult chat_completion_stream(
    const Config& config, 
    const std::vector<Message>& messages, 
    StreamCallback callback,
    const std::string& model_override = "",
    bool debug = false
);

// 去除字符串首尾空白字符
std::string trim(const std::string& str);

// 加载消息历史
std::optional<std::vector<Message>> load_messages(const std::filesystem::path& path);

// 保存消息历史
bool save_messages(
    const std::vector<Message>& messages, 
    const std::filesystem::path& path, 
    int max_history
);

// 清除消息历史
bool clear_messages(const std::filesystem::path& path);

// 显示消息历史
void show_messages(const std::filesystem::path& path);

} // namespace openai
} // namespace lc

// JSON序列化支持
namespace nlohmann {
template <>
struct adl_serializer<lc::openai::Message> {
    static void to_json(json& j, const lc::openai::Message& message);
    static void from_json(const json& j, lc::openai::Message& message);
};
}

#endif // LC_OPENAI_H