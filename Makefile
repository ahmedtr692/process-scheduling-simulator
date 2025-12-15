CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99
LDFLAGS = -lncurses

SRC_DIR = src
HDR_DIR = $(SRC_DIR)/headers
BUILD_DIR = build
BIN_DIR = bin
TARGET = $(BIN_DIR)/scheduler

# Core sources (always compiled)
CORE_SRCS = \
    $(SRC_DIR)/main.c \
    $(HDR_DIR)/basic.c \
    $(HDR_DIR)/config_parser.c \
    $(HDR_DIR)/display.c \
    $(HDR_DIR)/ncurses_display.c

# Optional algorithms
ALGORITHMS = fifo round_robin priority_preemptive multilevel multilevel_aging
ALG_SRCS = $(foreach a,$(ALGORITHMS),$(wildcard $(HDR_DIR)/$(a).c))

SRCS = $(CORE_SRCS) $(ALG_SRCS)
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete"
	@echo "Algorithms: $(notdir $(ALG_SRCS:.c=))"

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

run: $(TARGET)
	$(TARGET) examples/processes.txt

install: $(TARGET)
	install -d $(PREFIX)/bin
	install -m 755 $(TARGET) $(PREFIX)/bin/scheduler

uninstall:
	rm -f $(PREFIX)/bin/scheduler

.PHONY: all clean run install uninstall

