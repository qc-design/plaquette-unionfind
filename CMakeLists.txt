cmake_minimum_required(VERSION 3.20)

include(FetchContent)
  
project(plaquette_unionfind
    DESCRIPTION "C++ UnionFind Decoder"
    LANGUAGES CXX C
)

if (MSVC)
    # Explicitely allow exceptions
    add_compile_options(/EHa)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -mavx -mtune=native ${CMAKE_CXX_FLAGS}")



# Ensure the libraries can see additional libs at same level;
# Required for external deps when loading in Python
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(PLAQUETTE_UNIONFIND_ENABLE_CLANG_TIDY)
    if (NOT DEFINED CLANG_TIDY_BINARY)
        set(CLANG_TIDY_BINARY clang-tidy)
    endif()
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_BINARY};
                            -extra-arg=-std=c++20;
    )
endif()

if(PLAQUETTE_UNIONFIND_ENABLE_SANITIZER)
    add_compile_options(-fsanitize=address)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_link_options(-fsanitize=address)
    else ()
        add_link_options(-fsanitize=address -static-libasan)
    endif()
    set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
ENDIF()

add_subdirectory("plaquette_unionfind/src")

option(PLAQUETTE_FETCH_GRAPH_REPO "Use local plaquette_graph library. Set PLAQUETTE_GRAPH_INC_DIR if you are using this option." OFF)

if(NOT PLAQUETTE_FETCH_GRAPH_REPO)
  cmake_policy(SET CMP0135 OLD) # to suppress warning
  include(FetchContent)
  FetchContent_Declare(plaquette-graph
    URL ${CMAKE_SOURCE_DIR}/external/plaquette-graph.zip
  )
  FetchContent_MakeAvailable(plaquette-graph)
else()
  include(FetchContent)
  FetchContent_Declare(plaquette-graph
    GIT_REPOSITORY git@github.com:qc-design/plaquette-graph.git
    GIT_TAG        main
  )
  FetchContent_MakeAvailable(plaquette-graph)
endif()
set(PLAQUETTE_GRAPH_INC_DIR "${plaquette-graph_SOURCE_DIR}/plaquette_graph/src")

if(PLAQUETTE_UNIONFIND_BUILD_EXAMPLES)
  add_subdirectory("examples/cpp")
endif()

if (PLAQUETTE_UNIONFIND_BUILD_TESTS)
    add_subdirectory("plaquette_unionfind/src/tests")
endif()

if (PLAQUETTE_UNIONFIND_BUILD_MICRO_BENCHMARKS)
    add_subdirectory("plaquette_unionfind/src/micro_benchmarks")
endif()

if(PLAQUETTE_UNIONFIND_BUILD_BINDINGS)
# Ensure the libraries can see additional libs at same level;
# Required for external deps when loading in Python
# set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
# set(CMAKE_SKIP_BUILD_RPATH FALSE)
# set(CMAKE_POSITION_INDEPENDENT_CODE ON)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.10.1
)
FetchContent_MakeAvailable(pybind11)
find_package (Python COMPONENTS Interpreter Development)
pybind11_add_module(plaquette_unionfind_bindings "plaquette_unionfind/src/Bindings.cpp")
target_include_directories(plaquette_unionfind_bindings PUBLIC "${PLAQUETTE_GRAPH_INC_DIR}")
endif()
