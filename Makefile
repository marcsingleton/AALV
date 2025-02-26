SRC_DIR := src
TESTS_DIR := tests
BUILD_DIR := build

SRC := array.c formats.c
SRC_OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
SRC_TARGET := $(BUILD_DIR)/aalv

TESTS := test_array.c test_formats.c
TESTS_TARGET := $(TESTS:%.c=$(BUILD_DIR)/%)

# SRC rules
all: $(SRC_TARGET)

$(SRC_TARGET): $(SRC_OBJ) $(SRC_DIR)/main.c
	cc $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	cc -c $< -o $@

# TESTS rules
.PHONY: test
test: $(TESTS_TARGET)

$(BUILD_DIR)/test_%: $(TESTS_DIR)/test_%.c $(SRC_OBJ) | $(BUILD_DIR)
	cc $^ -I$(SRC_DIR) -o $@
	$@

# Utility rules
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
