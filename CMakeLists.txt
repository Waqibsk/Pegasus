cmake_minimum_required(VERSION 3.14)
project(pegasus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(pegasus_core INTERFACE)
target_include_directories(pegasus_core INTERFACE ${CMAKE_SOURCE_DIR}/src)
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)
