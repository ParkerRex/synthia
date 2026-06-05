cmake_minimum_required(VERSION 3.24)

if(APPLE AND (NOT DEFINED CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL ""))
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
endif()

project(Synth
    VERSION 0.1.0
    DESCRIPTION "Clean-room pluck instrument"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SYNTH_ENABLE_TESTS "Build Synth tests" ON)
option(SYNTH_ENABLE_COPY_AFTER_BUILD "Copy plugin bundles after build" OFF)
set(SYNTH_JUCE_GIT_TAG "8.0.13" CACHE STRING "JUCE git tag used when SYNTH_JUCE_PATH is not set")
set(SYNTH_JUCE_PATH "" CACHE PATH "Optional local JUCE checkout")

if(SYNTH_JUCE_PATH)
    add_subdirectory("${SYNTH_JUCE_PATH}" juce)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${SYNTH_JUCE_GIT_TAG}
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

set(SYNTH_COMMON_SOURCES
    src/dsp/Envelope.cpp
    src/dsp/Envelope.h
    src/dsp/Filter.cpp
    src/dsp/Filter.h
    src/dsp/Lfo.cpp
    src/dsp/Lfo.h
    src/dsp/OscillatorStack.cpp
    src/dsp/OscillatorStack.h
    src/dsp/Ramp.cpp
    src/dsp/Ramp.h
    src/dsp/SynthEngine.cpp
    src/dsp/SynthEngine.h
    src/dsp/SynthParameters.h
    src/plugin/ParameterRegistry.cpp
    src/plugin/ParameterRegistry.h
    src/presets/PresetManager.cpp
    src/presets/PresetManager.h
    src/presets/PresetValidator.cpp
    src/presets/PresetValidator.h
    src/voice/Voice.cpp
    src/voice/Voice.h
    src/voice/VoiceAllocator.cpp
    src/voice/VoiceAllocator.h)

juce_add_plugin(SynthPlugin
    COMPANY_NAME "ParkerX"
    BUNDLE_ID "com.parkerx.synth"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${SYNTH_ENABLE_COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE PkRx
    PLUGIN_CODE Syn1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Synth")

target_sources(SynthPlugin
    PRIVATE
        ${SYNTH_COMMON_SOURCES}
        src/plugin/PluginEditor.cpp
        src/plugin/PluginEditor.h
        src/plugin/PluginProcessor.cpp
        src/plugin/PluginProcessor.h)

target_compile_definitions(SynthPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        SYNTH_PROJECT_VERSION="${PROJECT_VERSION}"
        SYNTH_FACTORY_PRESET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/presets/factory")

target_link_libraries(SynthPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(SynthPlugin)

add_executable(SynthRender
    ${SYNTH_COMMON_SOURCES}
    src/validation/SynthRender.cpp)

target_link_libraries(SynthRender
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(SynthRender
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

if(SYNTH_ENABLE_TESTS)
    enable_testing()

    add_executable(SynthSmokeTest
        ${SYNTH_COMMON_SOURCES}
        tests/smoke/SynthSmokeTest.cpp)

    target_link_libraries(SynthSmokeTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SynthSmokeTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SynthSmokeTest COMMAND SynthSmokeTest)
    set_tests_properties(SynthSmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SynthContractTest
        ${SYNTH_COMMON_SOURCES}
        tests/smoke/SynthContractTest.cpp)

    target_link_libraries(SynthContractTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SynthContractTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SynthContractTest COMMAND SynthContractTest)
    set_tests_properties(SynthContractTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SynthVoiceCoreTest
        ${SYNTH_COMMON_SOURCES}
        tests/smoke/SynthVoiceCoreTest.cpp)

    target_link_libraries(SynthVoiceCoreTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SynthVoiceCoreTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SynthVoiceCoreTest COMMAND SynthVoiceCoreTest)
    set_tests_properties(SynthVoiceCoreTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SynthDspCoreTest
        ${SYNTH_COMMON_SOURCES}
        tests/smoke/SynthDspCoreTest.cpp)

    target_link_libraries(SynthDspCoreTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SynthDspCoreTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SynthDspCoreTest COMMAND SynthDspCoreTest)
    set_tests_properties(SynthDspCoreTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SynthRenderCoreSuite
        COMMAND SynthRender --suite core --output-dir build/reports/ctest-core)
    set_tests_properties(SynthRenderCoreSuite PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
