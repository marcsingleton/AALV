SRC_DIR := src
TESTS_DIR := tests
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
SRC_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
SRC_TARGET := $(BUILD_DIR)/aalv

TESTS := test_array.c test_fasta.c
TESTS_TARGETS := $(TESTS:%.c=$(BUILD_DIR)/%)

CC := cc
CFLAGS := -Wall -Wextra -pedantic -std=c99

# SRC rules
all: $(SRC_TARGET)

$(SRC_TARGET): $(SRC_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# TESTS rules
.PHONY: test
test: $(TESTS_TARGETS)

$(BUILD_DIR)/test_%: $(TESTS_DIR)/test_%.c $(SRC_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -I$(SRC_DIR) -o $@
	$@

# Utility rules
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
