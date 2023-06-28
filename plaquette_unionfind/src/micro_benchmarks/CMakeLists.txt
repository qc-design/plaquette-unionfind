cmake_minimum_required(VERSION 3.14)

project(micro_benchmarks)

set(CMAKE_CXX_STANDARD 20) # At least C++20 is required

message(STATUS "ENABLE_GB is ON. Find GBenchmark.")

include(FetchContent)

# Fetch GTest; required for GBenchmark
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.11.0 # latest
)
FetchContent_GetProperties(googletest)
set(BUILD_GMOCK OFF CACHE INTERNAL "")
set(INSTALL_GTEST OFF CACHE INTERNAL "")
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

# Fetch GBenchmark and surpress internal tests.
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.6.1 # latest
)
set(BENCHMARK_ENABLE_TESTING OFF)
FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
  FetchContent_Populate(googlebenchmark)
  add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR})
endif()

set(CMAKE_BUILD_TYPE Release)   

add_executable(grow_cluster_benchmark grow_cluster_benchmark.cpp)            
target_link_libraries(grow_cluster_benchmark
  PRIVATE
  benchmark::benchmark
  ${CMAKE_THREAD_LIBS_INIT}
)
