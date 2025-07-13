# Default target
.PHONY: all build compile tests test clean

# Default target builds the project
all: build

# Build the main project
build: compile

# Configure and build the project
compile:
	@mkdir -p build
	@cd build && cmake ..
	@cd build && cmake --build . --target compile

# Build tests
tests:
	@mkdir -p build
	@cd build && cmake ..
	@cd build && cmake --build . --target tests

# Run tests
test: tests
	@cd build && ctest --output-on-failure

# Clean build directory
clean:
	@rm -rf build
	@echo "Build directory cleaned"

# Help target
help:
	@echo "Available targets:"
	@echo "  build/compile - Build the main project (default)"
	@echo "  tests         - Build the test executable"
	@echo "  test          - Build and run tests"
	@echo "  clean         - Remove build directory"
	@echo "  help          - Show this help message"
