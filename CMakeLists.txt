cmake_minimum_required(VERSION 3.20)
project(MyProject)

# Set C++ standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Set optimization flags for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(NOT MSVC)
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
    endif()
endif()

# Find all source files in src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create main executable
add_executable(${PROJECT_NAME} ${SOURCES} main.cpp)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

target_link_libraries(${PROJECT_NAME}
  # add dependencies here
)



# Fetch and configure Google Test
include(FetchContent)
include(GoogleTest)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

# Fetch and configure Google Benchmark
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Enable testing
enable_testing()

# Manually specify test source files (subset of main sources needed for testing)
file(GLOB_RECURSE TEST_CPP_SOURCES
    # Add specific source files that need to be tested here
    "src/img_processing.cpp"
)

# Find all test files in test directory
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# Create test executable
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES} ${TEST_CPP_SOURCES})

# Link Google Test
target_link_libraries(${PROJECT_NAME}_tests gtest gtest_main)

# Include directories for tests
target_include_directories(${PROJECT_NAME}_tests PRIVATE include src)

# Automatically discover tests
gtest_discover_tests(${PROJECT_NAME}_tests)

# Benchmark configuration
file(GLOB_RECURSE BENCHMARK_SOURCES "benchmarks/*.cpp")

# Create benchmark executable
add_executable(${PROJECT_NAME}_benchmarks ${BENCHMARK_SOURCES} ${TEST_CPP_SOURCES})

# Link Google Benchmark
target_link_libraries(${PROJECT_NAME}_benchmarks benchmark::benchmark)

# Include directories for benchmarks
target_include_directories(${PROJECT_NAME}_benchmarks PRIVATE include src)

# Custom targets
add_custom_target(compile DEPENDS ${PROJECT_NAME})
add_custom_target(tests DEPENDS ${PROJECT_NAME}_tests)
add_custom_target(benchmarks DEPENDS ${PROJECT_NAME}_benchmarks)
