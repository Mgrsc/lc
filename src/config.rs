use anyhow::{Context, Result};
use serde::{Deserialize, Serialize};
use std::path::PathBuf;
use std::fs;

const DEFAULT_MAX_HISTORY: usize = 10;
const DEFAULT_SYSTEM_PROMPT: &str = "You are a professional Linux command-line assistant named lc. Your task is to answer users' questions about Linux commands, operations, and issues. Please follow these guidelines:

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

  Remember, you must check the requirements in the received Query and the information in the Input, and your response will be displayed directly on the command-line interface, so keep the format simple and the content clear.";

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Config {
    pub openai_api_key: String,
    pub openai_base_url: String,
    pub default_model: String,
    pub system_prompt: String,
    #[serde(default = "default_max_history")]
    pub max_history: usize,
}

fn default_max_history() -> usize {
    DEFAULT_MAX_HISTORY
}

impl Config {
    pub fn load() -> Result<Self> {
        let config_path = Self::config_path()?;
        if config_path.exists() {
            let config_str = fs::read_to_string(&config_path)
                .with_context(|| format!("Failed to read config file: {:?}", config_path))?;
            serde_yaml::from_str(&config_str).context("Failed to parse config file")
        } else {
            Ok(Self::default())
        }
    }

    pub fn save(&self) -> Result<()> {
        let config_path = Self::config_path()?;
        if let Some(parent) = config_path.parent() {
            fs::create_dir_all(parent).context("Failed to create config directory")?;
        }
        let config_str = serde_yaml::to_string(self).context("Failed to serialize config")?;
        fs::write(&config_path, config_str)
            .with_context(|| format!("Failed to write config file: {:?}", config_path))
    }

    pub fn set_value(&mut self, key: &str, value: &str) -> Result<()> {
        match key {
            "openai_api_key" => self.openai_api_key = value.to_string(),
            "openai_base_url" => self.openai_base_url = value.to_string(),
            "default_model" => self.default_model = value.to_string(),
            "max_history" => self.max_history = value.parse().context("Invalid max_history value")?,
            "system_prompt" => self.system_prompt = value.to_string(),
            _ => return Err(anyhow::anyhow!("Unknown config key: {}", key)),
        }
        self.save()
    }

    fn config_path() -> Result<PathBuf> {
        dirs::config_dir()
            .map(|mut path| {
                path.push("lc");
                path.push("config.yaml");
                path
            })
            .ok_or_else(|| anyhow::anyhow!("Failed to get config directory"))
    }
}

impl Default for Config {
    fn default() -> Self {
        Self {
            openai_api_key: String::new(),
            openai_base_url: "https://api.openai.com/v1".to_string(),
            default_model: "gpt-4o-mini".to_string(),
            system_prompt: DEFAULT_SYSTEM_PROMPT.to_string(),
            max_history: DEFAULT_MAX_HISTORY,
        }
    }
}

