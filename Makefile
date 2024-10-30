# Project name
PROJECT_NAME := lc

# Compilation target path
TARGET := target/release/$(PROJECT_NAME)

# Installation path
INSTALL_PATH := /usr/local/bin

# Default target
.PHONY: all
all: build

# Build the project
.PHONY: build
build:
	@echo "Building project..."
	@cargo build --release

# Install the command
.PHONY: install
install: build
	@echo "Installing $(PROJECT_NAME) to $(INSTALL_PATH)..."
	@sudo install -m 755 $(TARGET) $(INSTALL_PATH)

# Uninstall the command
.PHONY: uninstall
uninstall:
	@echo "Uninstalling $(PROJECT_NAME) from $(INSTALL_PATH)..."
	@sudo rm -f $(INSTALL_PATH)/$(PROJECT_NAME)

# Clean build artifacts
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@cargo clean

# Help information
.PHONY: help
help:
	@echo "Available make targets:"
	@echo "  all       - Default target, same as build"
	@echo "  build     - Build the project"
	@echo "  install   - Install the compiled program to $(INSTALL_PATH)"
	@echo "  uninstall - Uninstall the program"
	@echo "  clean     - Clean build artifacts"
	@echo "  help      - Display this help information"

