use anyhow::{Context, Result};
use reqwest::Client;
use serde::{Deserialize, Serialize};
use serde_json::{json, Value};

use crate::config::Config;

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Message {
    pub role: String,
    pub content: String,
}

pub async fn chat_completion(config: &Config, messages: Vec<Message>, debug: bool) -> Result<String> {
    let client = Client::new();
    let url = format!("{}/chat/completions", config.openai_base_url);

    if debug {
        println!("Sending request to: {}", url);
        println!(
            "Request payload: {}",
            json!({
                "model": config.default_model,
                "messages": messages,
            })
        );
    }

    let response = client
        .post(&url)
        .header("Authorization", format!("Bearer {}", config.openai_api_key))
        .json(&json!({
            "model": config.default_model,
            "messages": messages,
        }))
        .send()
        .await
        .context("Failed to send request")?;

    if debug {
        println!("Response status: {}", response.status());
    }

    let response_text = response.text().await.context("Failed to get response text")?;

    if debug {
        println!("Response body: {}", response_text);
    }

    let response_json: Value = serde_json::from_str(&response_text).context("Failed to parse JSON")?;

    response_json["choices"][0]["message"]["content"]
        .as_str()
        .map(|s| s.to_string())
        .ok_or_else(|| anyhow::anyhow!("Failed to extract content from response"))
}

