cmake_minimum_required(VERSION 3.13)
project(SynthTracker VERSION 1.1.7)

include(CTest)
enable_testing()

find_package(cmocka 2.0.2 CONFIG REQUIRED)

# Set the C standard
set(CMAKE_C_STANDARD 23)

# SDL2 Configuration using sdl-config
find_program(SDL_CONFIG sdl-config)
execute_process(COMMAND ${SDL_CONFIG} --cflags OUTPUT_VARIABLE SDL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND ${SDL_CONFIG} --libs OUTPUT_VARIABLE SDL_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

# Set compiler flags obtained from sdl-config
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SDL_CFLAGS}")


# OpenSSL Configuration (if required)
# find_package(OpenSSL REQUIRED)

# Endian Check
include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    add_definitions(-DWORDS_BIGENDIAN=1)
endif()

# Add the executable
add_executable(SynthTracker
    src/main.c
    src/application.c
    src/audio.c 
    src/player.c 
    src/tfmx.c
)
add_test(
    NAME test_synthtracker_cli_identity
    COMMAND $<TARGET_FILE:SynthTracker> -z
)
set_tests_properties(test_synthtracker_cli_identity PROPERTIES
    PASS_REGULAR_EXPRESSION "SynthTracker v1.1.7/SDL|Do not run SynthTracker as 'root'!"
    FAIL_REGULAR_EXPRESSION "tfmxplay|tfmx-play"
)
target_include_directories(SynthTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Compile probes for the affected production translation units.
add_library(tfmx_compile_probe OBJECT src/tfmx.c)
add_library(player_compile_probe OBJECT src/player.c)

set_target_properties(tfmx_compile_probe player_compile_probe PROPERTIES
    C_STANDARD 23
    C_EXTENSIONS OFF
)

target_include_directories(tfmx_compile_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_include_directories(player_compile_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_test(
    NAME tfmx_compile_probe
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tfmx_compile_probe
)
add_test(
    NAME player_compile_probe
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target player_compile_probe
)

add_executable(test_playback_context
    tests/playback/test_playback_context.c
    src/playback/playback_context.c
    src/playback/tfmx_loader.c
    src/playback/playback_legacy_bridge.c
    src/playback/playback_legacy_mixer.c
    src/player.c
)
target_include_directories(test_playback_context PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
)
target_compile_definitions(test_playback_context PRIVATE
    TFMX_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)
target_link_libraries(test_playback_context PRIVATE cmocka::cmocka)

add_test(
    NAME test_playback_context
    COMMAND test_playback_context
)

add_executable(test_application
    tests/application/test_application.c
    src/application.c
    src/tfmx.c
    src/player.c
    src/audio.c
)
target_include_directories(test_application PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(test_application PRIVATE
    cmocka::cmocka
    ${SDL_LIBS}
)

add_test(
    NAME test_application
    COMMAND test_application
)

# Link against the SDL2 library
target_link_libraries(SynthTracker ${SDL_LIBS})
