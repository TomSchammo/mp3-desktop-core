cmake_minimum_required(VERSION 3.20)
project(MP3Core)

# Set C++ standard to 23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

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
file(GLOB_RECURSE SOURCES "src/*.c")

# Create static library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    /opt/homebrew/opt/libpng/include/libpng16
    /opt/homebrew/opt/jpeg-turbo/include
    include
)

# Enable ARM NEON support for Apple Silicon and ARM processors
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ARM_NEON_AVAILABLE)
    if(APPLE)
        target_compile_options(${PROJECT_NAME} PRIVATE -mcpu=apple-m1)
    endif()
endif()

# Link libraries
find_library(PNG_LIBRARY png16 PATHS /opt/homebrew/opt/libpng/lib NO_DEFAULT_PATH)
find_library(JPEG_LIBRARY jpeg PATHS /opt/homebrew/opt/jpeg-turbo/lib NO_DEFAULT_PATH)

target_link_libraries(${PROJECT_NAME}
    ${PNG_LIBRARY}
    ${JPEG_LIBRARY}
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

# Find all test files in test directory
file(GLOB_RECURSE TEST_SOURCES "test/*.cpp")

# Create test executable
add_executable(${PROJECT_NAME}_tests ${TEST_SOURCES})

# Link Google Test and the library
target_link_libraries(${PROJECT_NAME}_tests
    gtest
    gtest_main
    ${PROJECT_NAME}
    ${PNG_LIBRARY}
    ${JPEG_LIBRARY}
)

# Include directories for tests
target_include_directories(${PROJECT_NAME}_tests PRIVATE
    /opt/homebrew/opt/libpng/include/libpng16
    /opt/homebrew/opt/jpeg-turbo/include
    include
    src
)

# Automatically discover tests
gtest_discover_tests(${PROJECT_NAME}_tests)

# Custom targets
add_custom_target(compile DEPENDS ${PROJECT_NAME})
add_custom_target(tests DEPENDS ${PROJECT_NAME}_tests)
