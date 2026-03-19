cmake_minimum_required(VERSION 3.20)
project (PersonaVox VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(PersonaVox
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    FORMATS Standalone VST3
    PRODUCT_NAME "PersonaVox"
    # Kaynak dosyaları buradan tek seferde bağlıyoruz
    SOURCES
        src/PluginProcessor.cpp
        src/PluginProcessor.h
)

target_compile_features(PersonaVox PRIVATE cxx_std_20)

target_link_libraries(PersonaVox
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
