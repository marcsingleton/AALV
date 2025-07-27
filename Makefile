# paths
SRC_DIR := src
TESTS_DIR := tests
BUILD_DIR := build
PROGRAM_NAME := aalv

# src targets
SRC := $(wildcard $(SRC_DIR)/*.c)
SRC := $(filter-out $(SRC_DIR)/main.c, $(SRC))
SRC_OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC))
SRC_TARGET := $(BUILD_DIR)/$(PROGRAM_NAME)

# tests targets
TESTS := $(wildcard $(TESTS_DIR)/*.c)
TESTS_DEPS := array.c fasta.c sequences.c str.c
TESTS_OBJS := $(TESTS_DEPS:%.c=$(BUILD_DIR)/%.o)
TESTS_TARGETS := $(TESTS:$(TESTS_DIR)/%.c=$(BUILD_DIR)/%)

# platform and program macros
OS := $(shell uname -s)
VERSION := 0.1.0
MACROS := -DPROGRAM_NAME=\"$(PROGRAM_NAME)\" -DVERSION=\"$(VERSION)\" # See macros.h for default values

# CC flags
CC := cc
CFLAGS := -Wall -Wextra -pedantic -std=c99
CPPFLAGS := $(MACROS)
ifeq ($(OS), Linux)
	CPPFLAGS += -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE -D_DEFAULT_SOURCE
endif

# src rules
all: $(SRC_TARGET)

$(SRC_TARGET): $(SRC_OBJS) $(SRC_DIR)/main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -lcurses -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# tests rules
.PHONY: test
test: $(TESTS_TARGETS)

$(BUILD_DIR)/test_%: $(TESTS_DIR)/test_%.c $(TESTS_OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $^ -I$(SRC_DIR) -o $@
	@echo
	$@
	@echo

# utility rules
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
