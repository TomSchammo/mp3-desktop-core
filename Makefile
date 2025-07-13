# Default target
.PHONY: all build compile tests test test-release benchmarks bench clean

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

# Run tests in Release mode (for performance verification)
test-release:
	@echo "Building and running tests in Release mode..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	@cd build && cmake --build . --target tests
	@cd build && ctest --output-on-failure

# Build benchmarks
benchmarks:
	@echo "Building benchmarks in Release mode..."
	@mkdir -p build
	@cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
	@cd build && cmake --build . --target benchmarks

# Run benchmarks with repetitions and aggregations (recommended for reliable results)
bench: benchmarks
	@echo "Running benchmarks (Release build, 5 repetitions with aggregations)..."
	@cd build && sudo ./MyProject_benchmarks --benchmark_repetitions=5 --benchmark_report_aggregates_only=true



# Clean build directory
clean:
	@rm -rf build
	@echo "Build directory cleaned"

# Help target
help:
	@echo "Available targets:"
	@echo "  build/compile - Build the main project (default)"
	@echo "  tests         - Build the test executable"
	@echo "  test          - Build and run tests (Debug mode)"
	@echo "  test-release  - Build and run tests (Release mode)"
	@echo "  benchmarks    - Build the benchmark executable (Release mode)"
	@echo "  bench         - Build and run benchmarks with repetitions (Release mode)"
	@echo "  clean         - Remove build directory"
	@echo "  help          - Show this help message"
