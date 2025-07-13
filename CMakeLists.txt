cmake_minimum_required(VERSION 3.20)
project(MyProject)

# Set C++ standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Enable testing
enable_testing()

# Manually specify test source files (subset of main sources needed for testing)
file(GLOB_RECURSE TEST_CPP_SOURCES
    # Add specific source files that need to be tested here
    # "src/file1.cpp"
    # "src/file2.cpp"
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

# Custom targets
add_custom_target(compile DEPENDS ${PROJECT_NAME})
add_custom_target(tests DEPENDS ${PROJECT_NAME}_tests)
