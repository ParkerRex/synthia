cmake_minimum_required(VERSION 3.24)

if(APPLE AND (NOT DEFINED CMAKE_OSX_ARCHITECTURES OR CMAKE_OSX_ARCHITECTURES STREQUAL ""))
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
endif()

project(SylenthAI
    VERSION 0.1.0
    DESCRIPTION "Sylenth AI lab instrument"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SYLENTH_AI_ENABLE_TESTS "Build sylenth-ai tests" ON)
option(SYLENTH_AI_ENABLE_COPY_AFTER_BUILD "Copy plugin bundles after build" OFF)
set(SYLENTH_AI_JUCE_GIT_TAG "8.0.13" CACHE STRING "JUCE git tag used when SYLENTH_AI_JUCE_PATH is not set")
set(SYLENTH_AI_JUCE_PATH "" CACHE PATH "Optional local JUCE checkout")

if(DEFINED SYNTH_ENABLE_TESTS)
    set(SYLENTH_AI_ENABLE_TESTS "${SYNTH_ENABLE_TESTS}" CACHE BOOL "Build sylenth-ai tests" FORCE)
endif()

if(DEFINED SYNTH_ENABLE_COPY_AFTER_BUILD)
    set(SYLENTH_AI_ENABLE_COPY_AFTER_BUILD "${SYNTH_ENABLE_COPY_AFTER_BUILD}" CACHE BOOL "Copy plugin bundles after build" FORCE)
endif()

if(DEFINED SYNTH_JUCE_GIT_TAG AND SYLENTH_AI_JUCE_GIT_TAG STREQUAL "8.0.13")
    set(SYLENTH_AI_JUCE_GIT_TAG "${SYNTH_JUCE_GIT_TAG}" CACHE STRING "JUCE git tag used when SYLENTH_AI_JUCE_PATH is not set" FORCE)
endif()

if(DEFINED SYNTH_JUCE_PATH AND SYLENTH_AI_JUCE_PATH STREQUAL "")
    set(SYLENTH_AI_JUCE_PATH "${SYNTH_JUCE_PATH}" CACHE PATH "Optional local JUCE checkout" FORCE)
endif()

if(SYLENTH_AI_JUCE_PATH)
    add_subdirectory("${SYLENTH_AI_JUCE_PATH}" juce)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${SYLENTH_AI_JUCE_GIT_TAG}
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

set(SYLENTH_AI_COMMON_SOURCES
    src/dsp/Arpeggiator.cpp
    src/dsp/Arpeggiator.h
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
    src/dsp/fx/FxChain.cpp
    src/dsp/fx/FxChain.h
    src/midi/MidiControllerMap.cpp
    src/midi/MidiControllerMap.h
    src/modulation/ModulationRouteModel.cpp
    src/modulation/ModulationRouteModel.h
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

juce_add_plugin(SylenthAIPlugin
    COMPANY_NAME "ParkerX"
    BUNDLE_ID "com.parkerx.sylenth-ai"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${SYLENTH_AI_ENABLE_COPY_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE PkRx
    PLUGIN_CODE SyAI
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "sylenth-ai")

target_sources(SylenthAIPlugin
    PRIVATE
        ${SYLENTH_AI_COMMON_SOURCES}
        src/plugin/PluginEditor.cpp
        src/plugin/PluginEditor.h
        src/plugin/PluginProcessor.cpp
        src/plugin/PluginProcessor.h)

foreach(SYLENTH_AI_PLUGIN_FORMAT_TARGET IN ITEMS SylenthAIPlugin_AU SylenthAIPlugin_VST3 SylenthAIPlugin_Standalone)
    juce_add_bundle_resources_directory(${SYLENTH_AI_PLUGIN_FORMAT_TARGET} "${CMAKE_CURRENT_SOURCE_DIR}/presets/factory")
endforeach()

target_compile_definitions(SylenthAIPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
        SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(SylenthAIPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(SylenthAIPlugin)

add_executable(SylenthAIRender
    ${SYLENTH_AI_COMMON_SOURCES}
    src/validation/SynthRender.cpp)

target_link_libraries(SylenthAIRender
    PRIVATE
        juce::juce_core
        juce::juce_audio_basics
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(SylenthAIRender
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
        SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

if(SYLENTH_AI_ENABLE_TESTS)
    enable_testing()

    add_executable(SylenthAISmokeTest
        ${SYLENTH_AI_COMMON_SOURCES}
        tests/smoke/SynthSmokeTest.cpp)

    target_link_libraries(SylenthAISmokeTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SylenthAISmokeTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SylenthAISmokeTest COMMAND SylenthAISmokeTest)
    set_tests_properties(SylenthAISmokeTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SylenthAIContractTest
        ${SYLENTH_AI_COMMON_SOURCES}
        tests/smoke/SynthContractTest.cpp)

    target_link_libraries(SylenthAIContractTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SylenthAIContractTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SylenthAIContractTest COMMAND SylenthAIContractTest)
    set_tests_properties(SylenthAIContractTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SylenthAIVoiceCoreTest
        ${SYLENTH_AI_COMMON_SOURCES}
        tests/smoke/SynthVoiceCoreTest.cpp)

    target_link_libraries(SylenthAIVoiceCoreTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SylenthAIVoiceCoreTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SylenthAIVoiceCoreTest COMMAND SylenthAIVoiceCoreTest)
    set_tests_properties(SylenthAIVoiceCoreTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_executable(SylenthAIDspCoreTest
        ${SYLENTH_AI_COMMON_SOURCES}
        tests/smoke/SynthDspCoreTest.cpp)

    target_link_libraries(SylenthAIDspCoreTest
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_processors
            juce::juce_dsp
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SylenthAIDspCoreTest
        PRIVATE
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            SYLENTH_AI_PROJECT_VERSION="${PROJECT_VERSION}"
            SYNTH_PROJECT_VERSION="${PROJECT_VERSION}")

    add_test(NAME SylenthAIDspCoreTest COMMAND SylenthAIDspCoreTest)
    set_tests_properties(SylenthAIDspCoreTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SylenthAIRenderCoreSuite
        COMMAND SylenthAIRender --suite core --output-dir build/reports/ctest-core)
    set_tests_properties(SylenthAIRenderCoreSuite PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SylenthAIRandomizeRenderTest
        COMMAND SylenthAIRender --randomize-test --output build/reports/ctest-randomize.json)
    set_tests_properties(SylenthAIRandomizeRenderTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SylenthAIModulationRouteRenderTest
        COMMAND SylenthAIRender --modulation-route-render-test --output build/reports/ctest-modulation-route-render.json)
    set_tests_properties(SylenthAIModulationRouteRenderTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SylenthAIOfflineRealtimeCompareTest
        COMMAND SylenthAIRender --offline-realtime-compare-test --output build/reports/ctest-offline-realtime-compare.json)
    set_tests_properties(SylenthAIOfflineRealtimeCompareTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME SylenthAIPatchRecreationSuite
        COMMAND SylenthAIRender --suite patch-recreation --output-dir build/reports/ctest-patch-recreation)
    set_tests_properties(SylenthAIPatchRecreationSuite PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
