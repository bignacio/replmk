# replmk/CMakeLists.txt
cmake_minimum_required(VERSION 3.27)

project(replmk CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CXX_PROJECT_FLAGS
    -Wall
    -Wpedantic
    -Werror
    -Wextra
    -Wconversion
    -Wsign-conversion
    -Walloca
    -Wshadow
    -Wfloat-equal
    -Wswitch-enum
    -Wcast-qual
    -Wimplicit-fallthrough
    -Wundef
    -Wfloat-equal
)


set(CXX_PROJECT_DEBUG_FLAGS
    -O0
    -g3
)


set(CXX_PROJECT_RELEASE_FLAGS
    -O3
    -DNDEBUG
)



if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CXX_PROJECT_FLAGS ${CXX_PROJECT_DEBUG_FLAGS})
else()
    list(APPEND CXX_PROJECT_FLAGS ${CXX_PROJECT_RELEASE_FLAGS})
endif()

message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiling with flags '${CXX_PROJECT_FLAGS}'")
message(STATUS "Using CXX standard: ${CMAKE_CXX_STANDARD}")

#'clang-diagnostic-*,clang-analyzer-*,clang-diagnostic-*,clang-analyzer-*,clang-diagnostic-*,clang-analyzer-*,-*,readability-*,clang-analyzer-core.*,clang-analyzer-cplusplus.*,modernize-*,performance-*,concurrency-*,cppcoreguidelines-*,bugprone*,-modernize-use-nodiscard,-performance-avoid-endl'
# Set global clang-tidy parameters
set(CLANG_TIDY_PARAMS "-p" "build" "-checks=clang-*,readability-*,modernize-*,performance-*,concurrency-*,cppcoreguidelines-*,bugprone*,-modernize-use-nodiscard,-performance-avoid-endl,-cppcoreguidelines-avoid-do-while" "-warnings-as-errors=*")

option(ENABLE_CLANG_TIDY "Enable Clang-Tidy static analysis" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY NAMES clang-tidy)
    if(CLANG_TIDY)
        message(STATUS "Clang-Tidy found: ${CLANG_TIDY}. Static analysis is available.")
    else()
        message(WARNING "Clang-Tidy not found, sorry.")
    endif()
endif()

include(FetchContent)
include(ExternalProject)


# ftxui for terminal UI components
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG        v6.1.9
)
FetchContent_MakeAvailable(ftxui)


# yaml-cpp for yaml parsing
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(yaml-cpp)

# cxxopts for command line options parsing
FetchContent_Declare(
    cxxopts
    GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
    GIT_TAG        v3.3.1
)
FetchContent_MakeAvailable(cxxopts)

# doctest for testing
SET(DOCTEST_WITH_MAIN ON)
SET(DOCTEST_WITH_COLORS ON)
SET(DOCTEST_WITH_LTO OFF)
SET(DOCTEST_WITH_TESTS OFF)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.12
)
FetchContent_MakeAvailable(doctest)

# Add subdirectories for source code and tests
add_subdirectory(src)
add_subdirectory(tests)
