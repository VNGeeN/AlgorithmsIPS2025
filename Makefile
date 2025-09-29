# Makefile for AlgosIPS2025 project

PROJECT_NAME = AlgosIPS2025
BUILD_DIR = build
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic

.PHONY: all debug release clean configure build run help rebuild examples test

# Default target
all: configure build

# Configure CMake
configure:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build project
build:
	@cd $(BUILD_DIR) && make -j$(shell nproc)

# Debug build
debug:
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j$(shell nproc)

# Release build
release:
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j$(shell nproc)

# Run main application
run: build
	@echo "Usage: ./$(BUILD_DIR)/$(PROJECT_NAME) <input_file>"

# Run examples
example1: build
	@./$(BUILD_DIR)/example1

example2: build
	@./$(BUILD_DIR)/example2

test_errors: build
	@./$(BUILD_DIR)/error_tests

# Clean build files
clean:
	@rm -rf $(BUILD_DIR)

# Clean and rebuild
rebuild: clean all

# Build and run all examples
examples: example1 example2 test_errors

# Help
help:
	@echo "Available targets:"
	@echo "  all         - Configure and build (default)"
	@echo "  debug       - Build with debug flags"
	@echo "  release     - Build with release flags"
	@echo "  build       - Build only"
	@echo "  run         - Show usage for main app"
	@echo "  example1    - Run basic usage example"
	@echo "  example2    - Run interactive example"
	@echo "  test_errors - Run arithmetic error tests"
	@echo "  examples    - Run all examples"
	@echo "  clean       - Remove build files"
	@echo "  rebuild     - Clean and rebuild"