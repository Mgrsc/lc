# LC - Linux Command Line LLM Assistant

<div align="center">
  <img src="./lc.jpeg" alt="LC Logo"/>
  <p><strong>智能命令行助手，为Linux终端注入AI能力</strong></p>
</div>

LC是一个轻量级的命令行工具，可以在终端中直接访问Large Language Models (LLM)，为您解答Linux命令、操作和问题。无需离开终端，即可获得专业的Linux支持。

## ✨ 主要特性

- **命令行原生体验**：直接在终端中获取AI回答，无需切换到浏览器或其他应用
- **实时响应**：支持流式输出，即时显示AI回答
- **会话记忆**：可选择保存对话历史，实现连续交流
- **管道支持**：可结合其他命令使用（例如：`ls -la | lc "解释这些文件权限"`)
- **可定制**：支持自定义模型、系统提示和API端点
- **高度可配置**：支持OpenAI兼容的API（包括各种开源或本地部署的替代品）

## 🚀 安装

### 预编译二进制（推荐）

```bash
# 下载最新版本
curl -LO https://github.com/username/lc/releases/latest/download/lc-linux-x86_64.tar.gz

# 解压并安装
tar -xzf lc-linux-x86_64.tar.gz
sudo mv lc /usr/local/bin/
```

### 从源码编译

需要先安装依赖：CMake (3.14+)、C++17兼容编译器、OpenSSL

```bash
# 克隆仓库
git clone https://github.com/username/lc.git
cd lc

# 编译
mkdir build && cd build
cmake ..
make

# 安装
sudo cp lc /usr/local/bin/
```

## ⚡ 快速开始

首次使用前，需要设置您的API密钥：

```bash
lc --set openai_api_key=sk-your_openai_api_key_here
```

然后就可以开始提问：

```bash
# 基本查询
lc "如何在Linux中查找大文件?"

# 结合其他命令使用
ls -la | lc "这些文件中哪些需要注意权限问题?"
```

## 📖 使用说明

### 基本命令格式

```bash
lc [OPTIONS] [QUERY]
```

### 选项

| 选项 | 描述 |
|------|------|
| `-q, --query <QUERY>` | 指定查询内容 (也可以直接作为参数提供) |
| `-m, --memory` | 启用会话记忆功能 |
| `--clear-memory` | 清除保存的会话记忆 |
| `--show-memory` | 显示当前保存的会话记忆 |
| `--model <MODEL>` | 为本次请求覆盖默认模型 |
| `--no-system-prompt` | 禁用系统提示 |
| `--set <KEY=VALUE>` | 设置配置项 |
| `--show-config` | 显示当前配置 |
| `--reset-config` | 重置配置为默认值 |
| `--debug` | 启用调试模式 |
| `-h, --help` | 显示帮助信息 |

### 配置选项

可通过`--set`修改的配置项：

| 配置项 | 描述 | 默认值 |
|-------|------|-------|
| `openai_api_key` | OpenAI API密钥 | (空) |
| `openai_base_url` | API基础URL | https://api.openai.com/v1 |
| `default_model` | 默认使用的模型 | gpt-4o-mini |
| `max_history` | 记忆模式下保存的最大消息数 | 10 |
| `system_prompt` | 系统提示内容 | (预设的Linux助手提示) |
| `use_system_prompt` | 是否使用系统提示 | true |

## 💡 使用示例

### 基本查询

```bash
# 请求帮助
lc "如何使用find命令查找最近24小时内修改的文件？"

# 解释命令
lc "解释这个命令：sed -i 's/foo/bar/g' *.txt"
```

### 使用上下文

```bash
# 通过管道提供上下文
cat error.log | lc "这个错误是什么意思？如何修复？"

# 查看系统信息并获取分析
uname -a | lc "告诉我这是什么版本的Linux以及它的主要特点"
```

### 连续对话

```bash
# 第一个问题
lc -m "如何设置SSH密钥？"

# 后续问题
lc -m "如何将这个密钥添加到GitHub？"
```

### 自定义模型

```bash
# 使用不同模型
lc --model gpt-4 "解释swap分区的作用和最佳大小设置"
```

## ⚙️ 配置文件

配置保存在 `~/.config/lc/config.yaml`，格式如下：

```yaml
openai_api_key: sk-your_api_key_here
openai_base_url: https://api.openai.com/v1
default_model: gpt-4o-mini
max_history: 10
use_system_prompt: true
system_prompt: |
  You are a professional Linux command-line assistant...
```

## 🔧 故障排除

### 常见问题

1. **API密钥错误**：确保使用了有效的API密钥 `lc --set openai_api_key=sk-...`

2. **网络问题**：如果使用代理，请确保您的网络环境可以访问API端点

3. **自托管模型**：使用本地或自托管API时，请设置正确的base_url：
   ```bash
   lc --set openai_base_url=http://localhost:8000/v1
   ```

## 🤝 贡献

欢迎贡献！请随时提交问题报告、功能请求或PR。

1. Fork项目
2. 创建您的分支 (`git checkout -b feature/amazing-feature`)
3. 提交您的更改 (`git commit -m 'Add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 开启Pull Request

## 📄 许可证

MIT License - 详见 [LICENSE](LICENSE) 文件