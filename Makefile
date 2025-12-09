# Process Scheduling Simulator Makefile
# GNU GPL v3.0 License

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = 

# Directories
SRC_DIR = src
HEADERS_DIR = $(SRC_DIR)/headers
BUILD_DIR = build
BIN_DIR = bin

# Target executable
TARGET = $(BIN_DIR)/scheduler

# Source files
SOURCES = $(SRC_DIR)/main.c \
          $(HEADERS_DIR)/basic.c \
          $(HEADERS_DIR)/fifo.c \
          $(HEADERS_DIR)/round_robin.c \
          $(HEADERS_DIR)/priority_preemptive.c \
          $(HEADERS_DIR)/multilevel.c \
          $(HEADERS_DIR)/multilevel_aging.c \
          $(HEADERS_DIR)/config_parser.c \
          $(HEADERS_DIR)/display.c

# Object files
OBJECTS = $(BUILD_DIR)/main.o \
          $(BUILD_DIR)/basic.o \
          $(BUILD_DIR)/fifo.o \
          $(BUILD_DIR)/round_robin.o \
          $(BUILD_DIR)/priority_preemptive.o \
          $(BUILD_DIR)/multilevel.o \
          $(BUILD_DIR)/multilevel_aging.o \
          $(BUILD_DIR)/config_parser.o \
          $(BUILD_DIR)/display.o

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build target
$(TARGET): $(BUILD_DIR) $(BIN_DIR) $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c $(HEADERS_DIR)/basic_sched.h $(HEADERS_DIR)/config_parser.h $(HEADERS_DIR)/display.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/main.c -o $(BUILD_DIR)/main.o

# Compile basic.c
$(BUILD_DIR)/basic.o: $(HEADERS_DIR)/basic.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/basic.c -o $(BUILD_DIR)/basic.o

# Compile fifo.c
$(BUILD_DIR)/fifo.o: $(HEADERS_DIR)/fifo.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/fifo.c -o $(BUILD_DIR)/fifo.o

# Compile round_robin.c
$(BUILD_DIR)/round_robin.o: $(HEADERS_DIR)/round_robin.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/round_robin.c -o $(BUILD_DIR)/round_robin.o

# Compile priority_preemptive.c
$(BUILD_DIR)/priority_preemptive.o: $(HEADERS_DIR)/priority_preemptive.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/priority_preemptive.c -o $(BUILD_DIR)/priority_preemptive.o

# Compile multilevel.c
$(BUILD_DIR)/multilevel.o: $(HEADERS_DIR)/multilevel.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/multilevel.c -o $(BUILD_DIR)/multilevel.o

# Compile multilevel_aging.c
$(BUILD_DIR)/multilevel_aging.o: $(HEADERS_DIR)/multilevel_aging.c $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/multilevel_aging.c -o $(BUILD_DIR)/multilevel_aging.o

# Compile config_parser.c
$(BUILD_DIR)/config_parser.o: $(HEADERS_DIR)/config_parser.c $(HEADERS_DIR)/config_parser.h $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/config_parser.c -o $(BUILD_DIR)/config_parser.o

# Compile display.c
$(BUILD_DIR)/display.o: $(HEADERS_DIR)/display.c $(HEADERS_DIR)/display.h $(HEADERS_DIR)/basic_sched.h
	$(CC) $(CFLAGS) -c $(HEADERS_DIR)/display.c -o $(BUILD_DIR)/display.o

# Install target
install: $(TARGET)
	@echo "Installing process scheduling simulator..."
	@if [ -w $(PREFIX) ]; then \
		install -d $(BINDIR); \
		install -m 755 $(TARGET) $(BINDIR)/scheduler; \
		echo "Installation complete: $(BINDIR)/scheduler"; \
	else \
		echo "Error: No write permission to $(PREFIX)"; \
		echo "Please run 'make install' with sudo or set PREFIX to a writable directory:"; \
		echo "  make install PREFIX=$$HOME/.local"; \
		exit 1; \
	fi

# Uninstall target
uninstall:
	@echo "Uninstalling process scheduling simulator..."
	@if [ -w $(PREFIX) ]; then \
		rm -f $(BINDIR)/scheduler; \
		echo "Uninstall complete"; \
	else \
		echo "Error: No write permission to $(PREFIX)"; \
		echo "Please run 'make uninstall' with sudo"; \
		exit 1; \
	fi

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# Clean and rebuild
rebuild: clean all

# Run the program (requires a config file)
run: $(TARGET)
	@if [ -f examples/processes.txt ]; then \
		$(TARGET) examples/processes.txt; \
	else \
		echo "Error: No configuration file found."; \
		echo "Create examples/processes.txt or run: $(TARGET) <config_file>"; \
	fi

# Help target
help:
	@echo "Process Scheduling Simulator - Makefile Help"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build the scheduler (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  rebuild   - Clean and rebuild"
	@echo "  install   - Install to $(PREFIX)/bin (may require sudo)"
	@echo "  uninstall - Remove installed files (may require sudo)"
	@echo "  run       - Build and run with example configuration"
	@echo "  help      - Display this help message"
	@echo ""
	@echo "Installation options:"
	@echo "  make install              - Install to /usr/local/bin (requires sudo)"
	@echo "  make install PREFIX=~/.local - Install to user directory"
	@echo ""

.PHONY: all clean rebuild install uninstall run help
