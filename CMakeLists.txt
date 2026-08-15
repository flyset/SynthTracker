cmake_minimum_required(VERSION 3.10)
project(tfmxplay VERSION 1.1.7)

include(CTest)
enable_testing()

# Set the C standard
set(CMAKE_C_STANDARD 23)

# SDL2 Configuration using sdl-config
find_program(SDL_CONFIG sdl-config)
execute_process(COMMAND ${SDL_CONFIG} --cflags OUTPUT_VARIABLE SDL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${SDL_CONFIG} --libs OUTPUT_VARIABLE SDL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Set compiler flags obtained from sdl-config
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SDL_CFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SDL_LIBS}")


# OpenSSL Configuration (if required)
# find_package(OpenSSL REQUIRED)

# Endian Check
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    add_definitions(-DWORDS_BIGENDIAN=1)
endif()

# Include directories for SDL2 and other headers
include_directories(include/)

# Add the executable
add_executable(tfmx 
    src/audio.c 
    src/player.c 
    src/tfmx.c
)

# Compile probes for the affected production translation units.
add_library(tfmx_compile_probe OBJECT src/tfmx.c)
add_library(player_compile_probe OBJECT src/player.c)

set_target_properties(tfmx_compile_probe player_compile_probe PROPERTIES
    C_STANDARD 23
    C_EXTENSIONS OFF
)

target_include_directories(tfmx_compile_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(player_compile_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(
    NAME tfmx_compile_probe
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tfmx_compile_probe
)
add_test(
    NAME player_compile_probe
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target player_compile_probe
)

# Link against the SDL2 library
target_link_libraries(tfmx ${SDL_LIBS})
