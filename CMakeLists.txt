cmake_minimum_required(VERSION 3.22)
project(Bopper VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build Universal Binary for both Intel and Apple Silicon
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")

# Add JUCE
add_subdirectory(JUCE)

# Configure plugin
juce_add_plugin(Bopper
    VERSION 1.0.0
    COMPANY_NAME "Bopper"
    PLUGIN_MANUFACTURER_CODE Bopr
    PLUGIN_CODE Bppr
    FORMATS AU Standalone
    PRODUCT_NAME "Bopper"

    # Plugin characteristics
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Auto-install after build
    COPY_PLUGIN_AFTER_BUILD TRUE

    # AU-specific - using Effect type since we don't need MIDI
    AU_MAIN_TYPE kAudioUnitType_Effect
)

# Source files
target_sources(Bopper PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GIF/GifLoader.cpp
    Source/GIF/GifAnimator.cpp
    Source/UI/BopperLookAndFeel.cpp
    Source/UI/GifDisplayComponent.cpp
    Source/UI/GifSelectorComponent.cpp
    Source/Utils/BpmSync.cpp
    Libs/EasyGifReader/EasyGifReader.cpp
    Libs/giflib/dgif_lib.c
    Libs/giflib/gifalloc.c
    Libs/giflib/gif_err.c
    Libs/giflib/gif_hash.c
    Libs/giflib/openbsd-reallocarray.c
)

target_include_directories(Bopper PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Libs
    ${CMAKE_SOURCE_DIR}/Libs/giflib
    ${CMAKE_SOURCE_DIR}/Libs/EasyGifReader
)

target_link_libraries(Bopper PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_graphics
    juce::juce_gui_basics
)

target_compile_definitions(Bopper PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Generate JuceHeader.h
juce_generate_juce_header(Bopper)

# Embed preset GIFs as binary resources
juce_add_binary_data(BopperBinaryData
    SOURCES
        gifs/spongebob.gif
        gifs/gandalf.gif
        "gifs/Dance Band GIF.gif"
)

target_link_libraries(Bopper PRIVATE BopperBinaryData)

# Silence some warnings from giflib
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set_source_files_properties(
        Libs/giflib/dgif_lib.c
        Libs/giflib/gifalloc.c
        Libs/giflib/gif_err.c
        Libs/giflib/gif_hash.c
        Libs/giflib/openbsd-reallocarray.c
        PROPERTIES COMPILE_FLAGS "-Wno-implicit-function-declaration -Wno-int-conversion"
    )
endif()
