# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.10)

# Project's name
project(TFMX)

# Set the C standard
set(CMAKE_C_STANDARD 99)

# Set the build type to Debug
set(CMAKE_BUILD_TYPE Debug)

# Find the SDL2 library
find_package(SDL2 REQUIRED)

# Include the SDL2 headers and your include directories
include_directories(${SDL2_INCLUDE_DIRS} include/)

# Add the executable
add_executable(tfmx 
    src/audio.c 
    src/player.c 
    src/tfmx.c
)

# Link against the SDL2 library
target_link_libraries(tfmx ${SDL2_LIBRARIES})
