cmake_minimum_required(VERSION 3.20)
project(VoiceChanger VERSION 1.0.0 LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check for WebAssembly build
if(EMSCRIPTEN)
    set(WASM_BUILD TRUE)
    message(STATUS "Building for WebAssembly")
else()
    set(WASM_BUILD FALSE)
    message(STATUS "Building for desktop platform")
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Multimedia Widgets)

# Source files
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/voiceeffects.cpp
    src/audiopipeline.cpp
    src/circularbuffer.cpp
    src/soundpack.cpp
    src/recorder.cpp
    src/hotkeys.cpp
    src/effects.cpp
    src/animationwidget.cpp
    src/gloweffekt.cpp
    src/presetnotification.cpp
    src/loadouts.cpp
    src/loading.cpp
    src/deletesoundpack.cpp
    src/psola.cpp
)

set(HEADERS
    src/mainwindow.h
    src/voiceeffects.h
    src/audiopipeline.h
    src/circularbuffer.h
    src/soundpack.h
    src/recorder.h
    src/hotkeys.h
    src/effects.h
    src/animationwidget.h
    src/gloweffekt.h
    src/presetnotification.h
    src/loadouts.h
    src/loading.h
    src/deletesoundpack.h
    src/psola.h
)

# UI files
set(UI_FILES
    src/mainwindow.ui
    src/loading.ui
    src/presetnotification.ui
)

# Resource files
set(RESOURCES
    src/Soundpack.qrc
)

# Create executable
add_executable(VoiceChanger ${SOURCES} ${HEADERS} ${UI_FILES} ${RESOURCES})

# Qt6 libraries
target_link_libraries(VoiceChanger 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Multimedia 
    Qt6::Widgets
)

if(WASM_BUILD)
    # WebAssembly specific configuration
    
    # Compiler flags
    target_compile_options(VoiceChanger PRIVATE
        -O3 -flto -s WASM=1 -s USE_PTHREADS=1
    )
    
    # Linker flags
    set_target_properties(VoiceChanger PROPERTIES
        LINK_FLAGS "-s WASM=1 -s USE_PTHREADS=1 -O3 -flto \
                    -s ALLOW_MEMORY_GROWTH=1 \
                    -s INITIAL_MEMORY=64MB \
                    -s MAX_MEMORY=512MB \
                    -s EXPORTED_FUNCTIONS='[\"_main\", \"_processAudio\", \"_initAudio\"]' \
                    -s EXPORTED_RUNTIME_METHODS='[\"ccall\", \"cwrap\"]' \
                    -s MODULARIZE=1 \
                    -s EXPORT_NAME=\"VoiceChangerModule\" \
                    -s ENVIRONMENT=\"web,webview,worker\" \
                    -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1 \
                    -s FORCE_FILESYSTEM=1 \
                    -s IDBFS=1 \
                    -s LZ4=1 \
                    -s USE_PTHREADS=1 \
                    -s PTHREAD_POOL_SIZE=4 \
                    -s ASSERTIONS=1 \
                    -s DEMANGLE_SUPPORT=1"
    )
    
    # FFmpeg for WebAssembly
    set(FFMPEG_WASM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../ffmpeg-wasm-lib)
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_WASM_ROOT}/include)
    set(FFMPEG_LIBRARY_DIRS ${FFMPEG_WASM_ROOT}/lib)
    
    find_library(AVCODEC_LIB avcodec PATHS ${FFMPEG_LIBRARY_DIRS})
    find_library(AVFORMAT_LIB avformat PATHS ${FFMPEG_LIBRARY_DIRS})
    find_library(AVUTIL_LIB avutil PATHS ${FFMPEG_LIBRARY_DIRS})
    find_library(SWRESAMPLE_LIB swresample PATHS ${FFMPEG_LIBRARY_DIRS})
    find_library(SWSCALE_LIB swscale PATHS ${FFMPEG_LIBRARY_DIRS})
    
    target_link_libraries(VoiceChanger
        ${AVCODEC_LIB}
        ${AVFORMAT_LIB}
        ${AVUTIL_LIB}
        ${SWRESAMPLE_LIB}
        ${SWSCALE_LIB}
    )
    
    target_include_directories(VoiceChanger PRIVATE ${FFMPEG_INCLUDE_DIRS})
    
    # SoundTouch for WebAssembly
    set(SOUNDTOUCH_WASM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../soundtouch-wasm)
    set(SOUNDTOUCH_INCLUDE_DIRS ${SOUNDTOUCH_WASM_ROOT}/include)
    set(SOUNDTOUCH_LIBRARY_DIRS ${SOUNDTOUCH_WASM_ROOT}/lib)
    
    find_library(SOUNDTOUCH_LIB SoundTouch PATHS ${SOUNDTOUCH_LIBRARY_DIRS})
    target_link_libraries(VoiceChanger ${SOUNDTOUCH_LIB})
    target_include_directories(VoiceChanger PRIVATE ${SOUNDTOUCH_INCLUDE_DIRS})
    
    # Set executable suffix for WebAssembly
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    
else()
    # Desktop platform configuration
    
    if(WIN32)
        # Windows FFmpeg
        set(FFMPEG_WIN_PATH "D:/ffmpeg-master-latest-win64-lgpl-shared")
        target_link_libraries(VoiceChanger
            ${FFMPEG_WIN_PATH}/lib/libswresample.dll
            ${FFMPEG_WIN_PATH}/lib/libavformat.dll
            ${FFMPEG_WIN_PATH}/lib/libavcodec.dll
        )
        target_include_directories(VoiceChanger PRIVATE ${FFMPEG_WIN_PATH}/include)
        
        # Windows SoundTouch
        set(SOUNDTOUCH_WIN_PATH "D:/soundtouch_dll-2.3.3")
        target_link_libraries(VoiceChanger ${SOUNDTOUCH_WIN_PATH}/SoundTouchDLL_x64.lib)
        target_include_directories(VoiceChanger PRIVATE ${SOUNDTOUCH_WIN_PATH})
        
        # Windows specific libraries
        target_link_libraries(VoiceChanger user32 mf mfplat mfreadwrite ole32 ksuser)
        
    elseif(APPLE)
        # macOS configuration
        target_link_libraries(VoiceChanger 
            avcodec avformat avutil swresample soundtouch
        )
        target_link_libraries(VoiceChanger 
            "-framework CoreAudio" "-framework AudioToolbox"
        )
        target_include_directories(VoiceChanger PRIVATE /usr/local/include)
        
    else()
        # Linux configuration
        target_link_libraries(VoiceChanger 
            avcodec avformat avutil swresample soundtouch X11
        )
        target_include_directories(VoiceChanger PRIVATE /usr/include/ffmpeg /usr/include/soundtouch)
    endif()
endif()

# Platform-specific resources
if(WIN32)
    # Windows icon
    set_target_properties(VoiceChanger PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    # macOS icon
    set_target_properties(VoiceChanger PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_ICON_FILE "icon.icns"
    )
endif()

# Installation rules
install(TARGETS VoiceChanger
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
