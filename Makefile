SRC_DIR := src
BUILD_DIR := build

SRC := array.c formats.c main.c
OBJ := $(SRC:%.c=$(BUILD_DIR)/%.o)
TARGET := $(BUILD_DIR)/aalv

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	cc -c $< -o $@

$(TARGET): $(OBJ)
	cc $^ -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean
