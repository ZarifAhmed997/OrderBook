cmake_minimum_required(VERSION 3.16)
project(orderbook_sim VERSION 1.0 LANGUAGES CXX)

# ---------------------------
# Compiler / build settings
# ---------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Warnings (good hygiene)
if(MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------
# Core library
# ---------------------------
add_library(orderbook
  src/orderbook.cpp
)

target_include_directories(orderbook
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Optional: extra optimisations in Release
if(NOT MSVC)
  target_compile_options(orderbook PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# ---------------------------
# Apps (executables)
# ---------------------------
add_executable(orderbook_cli
  apps/cli.cpp
)
target_link_libraries(orderbook_cli PRIVATE orderbook)

add_executable(orderbook_bench
  apps/benchmark.cpp
)
target_link_libraries(orderbook_bench PRIVATE orderbook)

# ---------------------------
# Tests (simple executable tests)
# ---------------------------

add_executable(orderbook_test
    tests/test_orderbook.cpp
)

target_link_libraries(orderbook_test PRIVATE orderbook)

enable_testing()
add_test(NAME OrderBookTests COMMAND orderbook_test)