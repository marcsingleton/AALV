SRC_DIR := src
TESTS_DIR := tests
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(filter-out $(SRC_DIR)/main.c, $(SRCS))
SRC_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
SRC_TARGET := $(BUILD_DIR)/aalv

TESTS := $(wildcard $(TESTS_DIR)/*.c)
TESTS_DEPS := array.c fasta.c sequences.c
TESTS_OBJS := $(TESTS_DEPS:%.c=$(BUILD_DIR)/%.o)
TESTS_TARGETS := $(TESTS:$(TESTS_DIR)/%.c=$(BUILD_DIR)/%)

CC := cc
CFLAGS := -Wall -Wextra -pedantic -std=c99

# SRC rules
all: $(SRC_TARGET)

$(SRC_TARGET): $(SRC_OBJS) $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# TESTS rules
.PHONY: test
test: $(TESTS_TARGETS)

$(BUILD_DIR)/test_%: $(TESTS_DIR)/test_%.c $(TESTS_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -I$(SRC_DIR) -o $@
	@echo
	$@
	@echo

# Utility rules
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
