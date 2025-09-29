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
	@./$(BUILD_DIR)/$(PROJECT_NAME)

run_test: build
	@./$(BUILD_DIR)/$(PROJECT_NAME) tests/basic_operations.txt

# Run examples
example1: build
	@./$(BUILD_DIR)/example1

example2: build
	@./$(BUILD_DIR)/example2

test_errors: build
	@./$(BUILD_DIR)/error_tests

tests_dir:
	@mkdir -p tests
	@echo "Creating sample test files..."
	@echo "# Basic arithmetic operations test file" > tests/basic_operations.txt
	@echo "2 + 3 * 4" >> tests/basic_operations.txt
	@echo "(1 + 2) * 3" >> tests/basic_operations.txt
	@echo "10 - 4 / 2" >> tests/basic_operations.txt
	@echo "2 ^ 3 + 1" >> tests/basic_operations.txt
	@echo "# Mathematical functions test file" > tests/functions.txt
	@echo "SIN(0) + COS(0)" >> tests/functions.txt
	@echo "EXP(0)" >> tests/functions.txt
	@echo "SIN(3.14159) + 1" >> tests/functions.txt
	@echo "# Edge cases and error handling" > tests/edge_cases.txt
	@echo "-5 + 3" >> tests/edge_cases.txt
	@echo "1 + 2 * (3 - 4)" >> tests/edge_cases.txt
	@echo "Sample test files created in tests/ directory"

# Run examples (if they exist)
example1: build
	@if [ -f "./$(BUILD_DIR)/example1" ]; then \
		./$(BUILD_DIR)/example1; \
	else \
		echo "example1 not built"; \
	fi

example2: build
	@if [ -f "./$(BUILD_DIR)/example2" ]; then \
		./$(BUILD_DIR)/example2; \
	else \
		echo "example2 not built"; \
	fi

test_errors: build
	@if [ -f "./$(BUILD_DIR)/error_tests" ]; then \
		./$(BUILD_DIR)/error_tests; \
	else \
		echo "error_tests not built"; \
	fi

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