cmake_minimum_required(VERSION 3.20)
project(goodboy CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Point CMake at the Homebrew SDL3 install (covers both Apple Silicon and
# Intel default prefixes) so find_package(SDL3) can locate its CMake config.
list(APPEND CMAKE_PREFIX_PATH /opt/homebrew/opt/sdl3 /usr/local/opt/sdl3)

find_package(SDL3 REQUIRED CONFIG)

# CONFIGURE_DEPENDS re-checks this glob on every build (CMake 3.12+), so newly
# added .cpp files are picked up without manually re-running cmake.
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(goodboy ${SOURCES})
target_include_directories(goodboy PRIVATE header)
target_link_libraries(goodboy PRIVATE SDL3::SDL3)
target_compile_options(goodboy PRIVATE -Wall -Wextra)
