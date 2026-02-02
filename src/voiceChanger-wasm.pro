# WebAssembly specific configuration
wasm {
    # WebAssembly platform detection
    CONFIG += wasm
    
    # Qt modules for WebAssembly
    QT += core gui multimedia widgets multimedia
    
    # Emscripten compiler settings
    QMAKE_CXXFLAGS += -s WASM=1 -s USE_PTHREADS=1
    QMAKE_LFLAGS += -s WASM=1 -s USE_PTHREADS=1
    
    # Memory settings for WebAssembly
    QMAKE_LFLAGS += -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=64MB -s MAX_MEMORY=512MB
    
    # WebAssembly specific optimizations
    QMAKE_CXXFLAGS += -O3 -flto
    QMAKE_LFLAGS += -O3 -flto
    
    # FFmpeg for WebAssembly (using precompiled wasm libraries)
    # Download from: https://github.com/ffmpegwasm/ffmpeg.wasm
    FFMPEG_WASM_PATH = $$PWD/../ffmpeg-wasm-lib
    
    LIBS += -L$$FFMPEG_WASM_PATH/lib \
            -lavcodec \
            -lavformat \
            -lavutil \
            -lswresample \
            -lswscale
    
    INCLUDEPATH += $$FFMPEG_WASM_PATH/include
    
    # SoundTouch for WebAssembly
    # Need to compile SoundTouch with Emscripten
    SOUNDTOUCH_WASM_PATH = $$PWD/../soundtouch-wasm
    
    LIBS += -L$$SOUNDTOUCH_WASM_PATH/lib \
            -lSoundTouch
    
    INCLUDEPATH += $$SOUNDTOUCH_WASM_PATH/include
    
    # WebAssembly specific linker flags
    QMAKE_LFLAGS += \
        -s EXPORTED_FUNCTIONS="['_main', '_processAudio', '_initAudio']" \
        -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" \
        -s MODULARIZE=1 \
        -s EXPORT_NAME="VoiceChangerModule" \
        -s ENVIRONMENT=web,webview,worker
    
    # Audio context for WebAssembly
    QMAKE_LFLAGS += -s WEBGL2_BACKWARDS_COMPATIBILITY_EMULATION=1
    
    # File system access for WebAssembly
    QMAKE_LFLAGS += \
        -s FORCE_FILESYSTEM=1 \
        -s IDBFS=1 \
        -s LZ4=1 \
        -s ALLOW_MEMORY_GROWTH=1
    
    # Threading support
    QMAKE_LFLAGS += -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4
    
    # Error handling
    QMAKE_LFLAGS += -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1
}

# Regular desktop builds (existing configuration)
!wasm {
    # Your existing desktop configuration here
    QT += core gui multimedia widgets
    
    CONFIG += c++17
    
    # Desktop FFmpeg and SoundTouch configuration...
    # (your existing library paths)
}
