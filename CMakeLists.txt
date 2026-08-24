cmake_minimum_required(VERSION 3.13)
project(SynthTracker VERSION 1.1.7)

include(CTest)
enable_testing()

find_package(cmocka 2.0.2 CONFIG REQUIRED)

set(CMAKE_C_STANDARD 23)

set(SYNTHTRACKER_PLAYBACK_SOURCES
    src/playback/playback_context.c
    src/playback/tfmx_loader.c
    src/playback/playback_legacy_bridge.c
    src/playback/playback_legacy_mixer.c
    src/playback/playback_legacy_renderer.c
    src/player.c
)

set(SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES
    src/audio_output/adapters/coreaudio_adapter.c
    src/audio_output/adapters/coreaudio_facade.c
)
set(SYNTHTRACKER_COREAUDIO_FRAMEWORKS)
if(APPLE)
    list(APPEND SYNTHTRACKER_COREAUDIO_FRAMEWORKS
        "-framework AudioToolbox"
        "-framework AudioUnit"
        "-framework CoreAudio"
    )
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
    add_definitions(-DWORDS_BIGENDIAN=1)
endif()

set(SYNTHTRACKER_APPLICATION_SOURCES
    src/application.c
    src/audio_output/audio_output.c
    ${SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES}
    ${SYNTHTRACKER_PLAYBACK_SOURCES}
)

add_executable(SynthTracker
    src/main.c
    ${SYNTHTRACKER_APPLICATION_SOURCES}
)
add_test(
    NAME test_synthtracker_cli_identity
    COMMAND $<TARGET_FILE:SynthTracker> -z
)
set_tests_properties(test_synthtracker_cli_identity PROPERTIES
    PASS_REGULAR_EXPRESSION "SynthTracker v1.1.7|Do not run SynthTracker as 'root'!"
    FAIL_REGULAR_EXPRESSION "tfmxplay|tfmx-play|/SDL"
)
target_include_directories(SynthTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
    ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
)
target_link_libraries(SynthTracker PRIVATE
    ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
)

add_library(player_compile_probe OBJECT src/player.c)
set_target_properties(player_compile_probe PROPERTIES
    C_STANDARD 23
    C_EXTENSIONS OFF
)
target_include_directories(player_compile_probe PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
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
    ${SYNTHTRACKER_APPLICATION_SOURCES}
)
target_include_directories(test_application PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
    ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
)
target_link_libraries(test_application PRIVATE
    cmocka::cmocka
    ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
)
add_test(
    NAME test_application
    COMMAND test_application
)

add_executable(test_application_removed_options
    tests/application/test_application_removed_options.c
    ${SYNTHTRACKER_APPLICATION_SOURCES}
)
target_include_directories(test_application_removed_options PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
    ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
)
target_link_libraries(test_application_removed_options PRIVATE
    cmocka::cmocka
    ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
)
add_test(
    NAME test_application_removed_options
    COMMAND test_application_removed_options
)

add_executable(test_application_audio_lifecycle
    tests/application/test_application_audio_lifecycle.c
    tests/audio_output/fake_coreaudio_facade.c
    ${SYNTHTRACKER_APPLICATION_SOURCES}
)
target_include_directories(test_application_audio_lifecycle PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
    ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_output
)
target_compile_definitions(test_application_audio_lifecycle PRIVATE
    SYNTHTRACKER_APPLICATION_TEST
    TFMX_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
)
target_link_libraries(test_application_audio_lifecycle PRIVATE
    cmocka::cmocka
    ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
)
add_test(
    NAME test_application_audio_lifecycle
    COMMAND test_application_audio_lifecycle
)

add_executable(test_audio_output
    tests/audio_output/test_audio_output.c
    tests/audio_output/fake_coreaudio_facade.c
    src/audio_output/audio_output.c
    ${SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES}
)
target_include_directories(test_audio_output PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_output
)
target_compile_definitions(test_audio_output PRIVATE
    SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
)
target_link_libraries(test_audio_output PRIVATE
    cmocka::cmocka
    ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
)
add_test(
    NAME test_audio_output
    COMMAND test_audio_output
)

if(APPLE)
    add_executable(test_legacy_exact_renderer
        tests/audio_output/test_legacy_exact_renderer.c
        tests/audio_output/fake_coreaudio_facade.c
        src/audio_output/audio_output.c
        ${SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES}
        ${SYNTHTRACKER_PLAYBACK_SOURCES}
    )
    target_include_directories(test_legacy_exact_renderer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_output
        ${CMAKE_CURRENT_SOURCE_DIR}/src/playback
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/audio_output
    )
    target_compile_definitions(test_legacy_exact_renderer PRIVATE
        SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE
        TFMX_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
    )
    target_link_libraries(test_legacy_exact_renderer PRIVATE
        cmocka::cmocka
        ${SYNTHTRACKER_COREAUDIO_FRAMEWORKS}
    )
    add_test(
        NAME test_legacy_exact_renderer
        COMMAND test_legacy_exact_renderer
    )
endif()
