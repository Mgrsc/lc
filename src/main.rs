mod config;
mod openai;

use anyhow::{Context, Result};
use clap::{Parser, CommandFactory};
use config::Config;
use openai::{chat_completion, Message};
use std::io::{self, Read};
use std::path::Path;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    /// Specify the query for the AI (can also be provided without -q)
    #[arg(short, long)]
    query: Option<String>,

    /// Enable conversation memory
    #[arg(short, long)]
    memory: bool,

    /// Clear the conversation memory
    #[arg(long)]
    clear_memory: bool,

    /// Set a configuration value (valid keys: openai_api_key, openai_base_url, default_model, max_history, system_prompt)
    #[arg(long)]
    set: Option<String>,

    /// Enable debug mode
    #[arg(long)]
    debug: bool,

    #[arg(trailing_var_arg = true)]
    args: Vec<String>,
}

#[tokio::main]
async fn main() -> Result<()> {
    let cli = Cli::parse();
    let mut config = Config::load().context("Failed to load config")?;

    if cli.debug {
        println!("Debug mode enabled");
        println!("Loaded config: {:?}", config);
    }

    if cli.clear_memory {
        clear_memory()?;
        println!("Conversation memory has been cleared.");
        return Ok(());
    }

    if let Some(set_arg) = cli.set {
        set_config(&mut config, &set_arg)?;
        return Ok(());
    }

    let query = get_query(&cli)?;
    let input = get_input()?;

    if cli.debug {
        println!("Query: {}", query);
        println!("Input: {}", input);
    }

    let mut messages = vec![Message {
        role: "system".to_string(),
        content: config.system_prompt.clone(),
    }];

    if cli.memory {
        if let Ok(prev_messages) = load_messages() {
            messages.extend(prev_messages);
        }
    }

    if !query.is_empty() || !input.is_empty() {
        messages.push(Message {
            role: "user".to_string(),
            content: format!("{}\n\n{}", query, input).trim().to_string(),
        });
    }

    if cli.debug {
        println!("Messages: {:?}", messages);
    }

    match chat_completion(&config, messages.clone(), cli.debug).await {
        Ok(response) => {
            println!("{}", response);

            if cli.memory {
                messages.push(Message {
                    role: "assistant".to_string(),
                    content: response,
                });
                save_messages(&messages, config.max_history)?;
            }
        }
        Err(e) => eprintln!("Error: {}", e),
    }

    Ok(())
}

fn clear_memory() -> Result<()> {
    let memory_file = Path::new("conversation_memory.json");
    if memory_file.exists() {
        std::fs::remove_file(memory_file).context("Failed to remove memory file")?;
        println!("Conversation memory has been cleared.");
    } else {
        println!("No conversation memory found.");
    }
    Ok(())
}

fn save_messages(messages: &[Message], max_history: usize) -> Result<()> {
    let messages_to_save: Vec<Message> = messages
        .iter()
        .filter(|m| m.role != "system")
        .rev()
        .take(max_history)
        .cloned()
        .collect::<Vec<_>>()
        .into_iter()
        .rev()
        .collect();

    let serialized = serde_json::to_string(&messages_to_save).context("Failed to serialize messages")?;
    std::fs::write("conversation_memory.json", serialized).context("Failed to write memory file")?;
    Ok(())
}

fn load_messages() -> Result<Vec<Message>> {
    let content = std::fs::read_to_string("conversation_memory.json").context("Failed to read memory file")?;
    let messages: Vec<Message> = serde_json::from_str(&content).context("Failed to parse memory file")?;
    Ok(messages)
}

fn set_config(config: &mut Config, set_arg: &str) -> Result<()> {
    let parts: Vec<&str> = set_arg.splitn(2, '=').collect();
    if parts.len() == 2 {
        config.set_value(parts[0], parts[1])?;
        println!("{} set successfully.", parts[0]);
    } else {
        eprintln!("Invalid set argument. Use format: key=value");
    }
    Ok(())
}

fn get_query(cli: &Cli) -> Result<String> {
    Ok(if let Some(q) = &cli.query {
        format!("Query: {}", q.trim())
    } else if !cli.args.is_empty() {
        format!("Query: {}", cli.args.join(" ").trim())
    } else if cli.memory {
        String::new()
    } else {
        Cli::command().print_help()?;
        std::process::exit(0);
    })
}

fn get_input() -> Result<String> {
    if atty::is(atty::Stream::Stdin) {
        Ok(String::new())
    } else {
        let mut buffer = String::new();
        io::stdin().read_to_string(&mut buffer)?;
        Ok(format!("Input: {}", buffer.trim()))
    }
}

