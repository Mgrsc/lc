# **lc - Linux Command-line Assistant**

`lc` is a professional **Linux command-line assistant** designed to help users answer questions about Linux commands, operations, and problems.  It supports OpenAI-formatted LLM requests, provides concise and clear command-line answers, and supports a rich set of configuration options.

---

## **Features** ✨

- **Instant Query**: Query Linux command-related questions directly through the command line.
- **Conversation Memory**: Optionally enable conversation memory to save historical conversations for subsequent queries.
- **Configuration Management**: Supports setting and saving OpenAI API keys, models, history length, and other configurations.
- **Debug Mode**: Provides a debug mode that outputs detailed request and response information for easy debugging.

---

## **Installation** 📦

### **Compile and Install**

Use the following command to compile and install `lc`:

```bash
make install
```

### **Uninstall**

Use the following command to uninstall `lc`:

```bash
make uninstall
```

---

## **Usage** 🛠️

### **Basic Usage**

```bash
lc -q "How do I use the ls command to list all files?"
```

### **Enable Conversation Memory**

```bash
lc -q "How do I use the grep command?" -m
```

### **Clear Conversation Memory**

```bash
lc --clear-memory
```

### **Set Configuration Values**

```bash
lc --set openai_api_key=YOUR_API_KEY
```

### **Enable Debug Mode**

```bash
lc -q "How do I use the find command?" --debug
```

---

## **Configuration File** ⚙️

The configuration file is located at `~/.config/lc/config.yaml` and contains the following configuration items:

| Configuration Item | Description                                       |
| ---------------- | ------------------------------------------------- |
| `openai_api_key`  | OpenAI API Key                                  |
| `openai_base_url` | OpenAI API Base URL                              |
| `default_model`   | Default GPT Model                               |
| `system_prompt`   | System Prompt                                     |
| `max_history`     | Maximum History Length                           |

---

## **Makefile Targets** 📝

| Target       | Description                                                     |
| ---------- | --------------------------------------------------------------- |
| `all`      | Default target, equivalent to `build`                          |
| `build`    | Compile the project                                              |
| `install`  | Install the compiled program to `/usr/local/bin`               |
| `uninstall`| Uninstall the program                                              |
| `clean`    | Clean up compiled files                                          |
| `help`     | Display help information                                        |

---

## **Code Structure** 🏗️

- **`config.rs`**: Handles loading and saving the configuration file.
- **`main.rs`**: Main program entry point, handles command-line arguments and logic.
- **`openai.rs`**: Interaction logic with the OpenAI API.

---

## **Examples** 📖

### **Query Example**

```bash
lc -q "How do I use the tar command to extract files?"
```

### **Query Example with Conversation Memory Enabled**

```bash
lc -q "How do I use the awk command to process text?" -m
```

### **Configuration Example**

```bash
lc --set openai_api_key=YOUR_API_KEY
```

---

## **Contribution** 🤝

We welcome **Issues** and **Pull Requests** to contribute code and improve documentation. We look forward to your participation!

---

We hope this README helps you better use and understand the `lc` project! If you have any questions or suggestions, please feel free to provide feedback.