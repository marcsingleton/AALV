SRC_DIR := src
BUILD_DIR := build

all: $(SRC_DIR)/main.c build
	cc $(SRC_DIR)/main.c -o $(BUILD_DIR)/aalv

build:
	mkdir -p $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
