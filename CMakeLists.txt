cmake_minimum_required(VERSION 3.20)
project(TerminalNBodySim)
# -DCMAKE_BUILD_TYPE=Debug
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Release flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

find_package(OpenMP REQUIRED)

add_executable(sim main.cpp parser.cpp)

target_link_libraries(sim 
    OpenMP::OpenMP_CXX
)

target_include_directories(sim PRIVATE
    ${CMAKE_SOURCE_DIR}
)

include(FetchContent)

FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
#   GIT_TAG <yaml-cpp> # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)

target_link_libraries(sim yaml-cpp::yaml-cpp) # The library or executable that require yaml-cpp library