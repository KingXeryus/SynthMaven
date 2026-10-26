cmake_minimum_required(VERSION 3.22)
# Set the deployment target before enabling Apple compilers. An old empty cache
# must not silently inherit the build Mac's newest OS version.
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
 if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum Apple OS version" FORCE)
 else()
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum Apple OS version" FORCE)
 endif()
endif()
project(SynthMaven VERSION 0.4.0 LANGUAGES C CXX OBJCXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
 set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS version")
 set(SYNTHMAVEN_FORMATS AUv3 Standalone)
 file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Resources/iOS-Scenes.plist" SYNTHMAVEN_PLIST)
else()
 set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
 set(SYNTHMAVEN_FORMATS VST3 AU Standalone)
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(vendor/JUCE-7.0.12 EXCLUDE_FROM_ALL)
add_library(SynthMavenDSP STATIC ../build/desktop-generated/SynthMavenDSP.c)
target_compile_options(SynthMavenDSP PRIVATE -O2 -w)
target_include_directories(SynthMavenDSP PUBLIC Source)
juce_add_binary_data(SynthMavenAssets SOURCES Resources/SynthMaven-logo.png Resources/Banks/YSE-Originals.syx)
juce_add_plugin(SynthMaven
 ICON_BIG Resources/SynthMaven-icon.png
 COMPANY_NAME "SynthMaven" BUNDLE_ID "com.synthmaven.instrument"
 PLUGIN_MANUFACTURER_CODE Smvn PLUGIN_CODE SyMv
 FORMATS ${SYNTHMAVEN_FORMATS} PRODUCT_NAME "SynthMaven"
 IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
 COPY_PLUGIN_AFTER_BUILD FALSE
 AU_MAIN_TYPE kAudioUnitType_MusicDevice
 VST3_CATEGORIES Instrument Synth
 MICROPHONE_PERMISSION_ENABLED FALSE
 BLUETOOTH_PERMISSION_ENABLED TRUE
 BLUETOOTH_PERMISSION_TEXT "Connect Bluetooth MIDI keyboards and pedals to SynthMaven."
 BACKGROUND_AUDIO_ENABLED TRUE
 FILE_SHARING_ENABLED TRUE
 DOCUMENT_BROWSER_ENABLED TRUE
 PLIST_TO_MERGE "${SYNTHMAVEN_PLIST}")
target_sources(SynthMaven PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/PresetLibrary.cpp Source/ScaleTuning.cpp Source/SamplerEngine.cpp)
target_compile_definitions(SynthMaven PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)
target_link_libraries(SynthMaven PRIVATE SynthMavenDSP SynthMavenAssets juce::juce_audio_utils juce::juce_dsp PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
add_executable(SynthMavenDSPTests tests/DSPTests.c)
target_link_libraries(SynthMavenDSPTests PRIVATE SynthMavenDSP)
enable_testing()
add_test(NAME DSPIsolationAndPresets COMMAND SynthMavenDSPTests)
add_executable(SynthMavenProcessorTests tests/ProcessorTests.cpp)
target_include_directories(SynthMavenProcessorTests PRIVATE Source vendor/JUCE-7.0.12/modules)
target_compile_definitions(SynthMavenProcessorTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(SynthMavenProcessorTests PRIVATE SynthMaven)
add_test(NAME ProcessorRatesAndState COMMAND SynthMavenProcessorTests)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_executable(SynthMavenOriginalRender tests/OriginalDSP.c tests/RenderHash.c)
target_include_directories(SynthMavenOriginalRender PRIVATE Source ../build/gen ../firmware/src ../firmware/hal)
target_compile_definitions(SynthMavenOriginalRender PRIVATE SYNTHMAVEN_SYNTH=1 SM_BLE=0)
target_compile_options(SynthMavenOriginalRender PRIVATE -O2 -w)
add_executable(SynthMavenIsolatedRender tests/RenderHash.c)
target_link_libraries(SynthMavenIsolatedRender PRIVATE SynthMavenDSP)
add_test(NAME OriginalDSPEquivalence COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/CompareOriginal.py $<TARGET_FILE:SynthMavenOriginalRender> $<TARGET_FILE:SynthMavenIsolatedRender>)
add_executable(SynthMavenLayerMidiTests tests/LayerMidiTests.cpp)
target_include_directories(SynthMavenLayerMidiTests PRIVATE Source vendor/JUCE-7.0.12/modules)
target_compile_definitions(SynthMavenLayerMidiTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(SynthMavenLayerMidiTests PRIVATE SynthMaven)
add_test(NAME LayersMidiAndCatalog COMMAND SynthMavenLayerMidiTests)
add_executable(SynthMavenTuningTests tests/TuningTests.cpp)
target_include_directories(SynthMavenTuningTests PRIVATE Source vendor/JUCE-7.0.12/modules)
target_compile_definitions(SynthMavenTuningTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(SynthMavenTuningTests PRIVATE SynthMaven)
add_test(NAME ScalaAndMasterTuning COMMAND SynthMavenTuningTests)

add_executable(SynthMavenMixerSamplerTests tests/MixerSamplerTests.cpp)
target_include_directories(SynthMavenMixerSamplerTests PRIVATE Source vendor/JUCE-7.0.12/modules)
target_compile_definitions(SynthMavenMixerSamplerTests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
target_link_libraries(SynthMavenMixerSamplerTests PRIVATE SynthMaven)
add_test(NAME MixerSplitsAndSampler COMMAND SynthMavenMixerSamplerTests)
add_executable(SynthMavenAUBundleSmoke tests/AUBundleSmoke.cpp)
target_link_libraries(SynthMavenAUBundleSmoke PRIVATE "-framework AudioToolbox" "-framework CoreFoundation")
add_dependencies(SynthMavenAUBundleSmoke SynthMaven_AU)
add_test(NAME AudioUnitBundleRender COMMAND SynthMavenAUBundleSmoke $<TARGET_FILE:SynthMaven_AU>)
endif()
