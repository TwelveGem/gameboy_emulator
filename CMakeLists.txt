cmake_minimum_required(VERSION 3.20)
project(goodboy CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Point CMake at the Homebrew SDL3 install (covers both Apple Silicon and
# Intel default prefixes) so find_package(SDL3) can locate its CMake config.
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew/opt/sdl3 /usr/local/opt/sdl3)

find_package(SDL3 REQUIRED CONFIG)

add_executable(goodboy src/main.cpp src/cart.cpp src/cpu_instructions.cpp src/cpu.cpp)
target_include_directories(goodboy PRIVATE header)
target_link_libraries(goodboy PRIVATE SDL3::SDL3)
