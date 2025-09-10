# Makefile for Forth Interpreter

CC = gcc
CFLAGS = -std=c99
DEBUG_FLAGS = -g
SOURCES = forth.c forth.h
TARGET = forth
TEST_FILE = test.forth

.PHONY: all debug clean test format help

# Default target: build the interpreter
all: $(TARGET)

# Build the interpreter
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) forth.c -o $(TARGET)

# Build with debug flags
debug: $(SOURCES)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) forth.c -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Run tests using demo.fth
test: $(TARGET)
	./$(TARGET) < $(TEST_FILE)

# Format source code using clang-format (Microsoft style)
format:
	clang-format -i -style=Microsoft forth.c forth.h

# Show help
help:
	@echo "Available targets:"
	@echo "  all     - Build the interpreter (default)"
	@echo "  debug   - Build with debug flags"
	@echo "  clean   - Remove build artifacts"
	@echo "  test    - Run tests using $(TEST_FILE)"
	@echo "  format  - Format source code with clang-format"
	@echo "  help    - Show this help message"