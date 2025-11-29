# Process Scheduling Simulator Makefile
# Université Tunis El Manar - Institut Supérieur d'Informatique
# Mini Projet Systèmes d'exploitation 2025-2026

CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -g
LDFLAGS =

# Directories
SRC_DIR = src
HEADERS_DIR = $(SRC_DIR)/headers
BUILD_DIR = build
BIN_DIR = bin
POLICIES_DIR = policies

# Installation directories
PREFIX ?= /usr/local
INSTALL_BIN = $(PREFIX)/bin
INSTALL_SHARE = $(PREFIX)/share/scheduler

# Target
TARGET = scheduler
TARGET_PATH = $(BIN_DIR)/$(TARGET)

# Source files
MAIN_SRC = $(SRC_DIR)/main.c
SCHED_SRC = $(HEADERS_DIR)/basic_sched.c

# Object files
MAIN_OBJ = $(BUILD_DIR)/main.o
SCHED_OBJ = $(BUILD_DIR)/basic_sched.o
OBJECTS = $(MAIN_OBJ) $(SCHED_OBJ)

# Default target
.PHONY: all
all: directories $(TARGET_PATH)

# Create directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(POLICIES_DIR)

# Link
$(TARGET_PATH): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build successful: $(TARGET_PATH)"

# Compile main
$(MAIN_OBJ): $(MAIN_SRC) $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $< -o $@

# Compile scheduler
$(SCHED_OBJ): $(SCHED_SRC) $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)
	@echo "Clean complete"

# Install
.PHONY: install
install: all
	@echo "Installing $(TARGET)..."
	@if [ -w $(PREFIX) ]; then \
		mkdir -p $(INSTALL_BIN); \
		mkdir -p $(INSTALL_SHARE); \
		cp $(TARGET_PATH) $(INSTALL_BIN)/; \
		cp -r $(POLICIES_DIR) $(INSTALL_SHARE)/; \
		echo "Installation complete in $(PREFIX)"; \
	else \
		echo "No write permission to $(PREFIX)."; \
		echo "Installing to ~/bin instead..."; \
		mkdir -p $(HOME)/bin; \
		mkdir -p $(HOME)/.local/share/scheduler; \
		cp $(TARGET_PATH) $(HOME)/bin/; \
		cp -r $(POLICIES_DIR) $(HOME)/.local/share/scheduler/; \
		echo "Installation complete in $(HOME)/bin"; \
		echo "Add $(HOME)/bin to your PATH if not already done:"; \
		echo "  export PATH=\$$HOME/bin:\$$PATH"; \
	fi

# Uninstall
.PHONY: uninstall
uninstall:
	@if [ -w $(PREFIX) ]; then \
		rm -f $(INSTALL_BIN)/$(TARGET); \
		rm -rf $(INSTALL_SHARE); \
		echo "Uninstalled from $(PREFIX)"; \
	else \
		rm -f $(HOME)/bin/$(TARGET); \
		rm -rf $(HOME)/.local/share/scheduler; \
		echo "Uninstalled from $(HOME)/bin"; \
	fi

# Run with sample config
.PHONY: run
run: all
	@if [ -f sample_config.txt ]; then \
		./$(TARGET_PATH) sample_config.txt; \
	else \
		echo "No sample_config.txt found. Create one first."; \
		echo "Example format:"; \
		echo "# Process name arrival_time burst_time priority [quantum]"; \
		echo "P1 0 10 2"; \
		echo "P2 1 5 1"; \
		echo "P3 2 8 3"; \
	fi

# Debug build
.PHONY: debug
debug: CFLAGS += -DDEBUG -O0
debug: clean all

# Release build
.PHONY: release
release: CFLAGS += -O2 -DNDEBUG
release: clean all

# Help
.PHONY: help
help:
	@echo "Process Scheduling Simulator - Build System"
	@echo ""
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the scheduler (default)"
	@echo "  clean     - Remove build files"
	@echo "  install   - Install the scheduler"
	@echo "  uninstall - Uninstall the scheduler"
	@echo "  run       - Build and run with sample_config.txt"
	@echo "  debug     - Build with debug symbols"
	@echo "  release   - Build optimized release"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  PREFIX    - Installation prefix (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make"
	@echo "  make install PREFIX=/opt/scheduler"
	@echo "  make run"
