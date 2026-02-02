#!/bin/bash

# WebAssembly Build Script for VoiceChanger
echo "Building VoiceChanger for WebAssembly..."

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "Emscripten not found. Please install Emscripten SDK:"
    echo "git clone https://github.com/emscripten-core/emsdk.git"
    echo "cd emsdk"
    echo "emsdk install latest"
    echo "emsdk activate latest"
    echo "source ./emsdk_env.sh"
    exit 1
fi

# Create directories for WebAssembly libraries
mkdir -p ../ffmpeg-wasm-lib/lib
mkdir -p ../ffmpeg-wasm-lib/include
mkdir -p ../soundtouch-wasm/lib
mkdir -p ../soundtouch-wasm/include

echo "Downloading FFmpeg WebAssembly libraries..."

# Download precompiled FFmpeg WebAssembly libraries
if [ ! -f "../ffmpeg-wasm-lib/lib/libavcodec.a" ]; then
    echo "Building FFmpeg for WebAssembly..."
    cd ../
    
    if [ ! -d "ffmpeg" ]; then
        git clone https://github.com/FFmpeg/FFmpeg.git
    fi
    
    cd FFmpeg
    emconfigure ./configure \
        --prefix=../ffmpeg-wasm-lib \
        --disable-programs \
        --disable-doc \
        --disable-ffmpeg \
        --disable-ffplay \
        --disable-ffprobe \
        --disable-avdevice \
        --disable-swresample \
        --disable-swscale \
        --disable-postproc \
        --disable-avfilter \
        --disable-everything \
        --enable-avcodec \
        --enable-avformat \
        --enable-avutil \
        --enable-decoder=aac \
        --enable-decoder=mp3 \
        --enable-decoder=wav \
        --enable-encoder=aac \
        --enable-encoder=libmp3lame \
        --enable-muxer=mp4 \
        --enable-muxer=wav \
        --enable-demuxer=mp3 \
        --enable-demuxer=wav \
        --enable-protocol=file \
        --enable-cross-compile \
        --target-os=none \
        --arch=x86_32 \
        --cpu=generic \
        --disable-runtime-cpudetect \
        --disable-pic \
        --disable-shared \
        --enable-static \
        --disable-symver \
        --disable-asm \
        --disable-fast-unaligned
    
    emmake make -j4
    emmake make install
    cd ../src
fi

echo "Building SoundTouch for WebAssembly..."

if [ ! -f "../soundtouch-wasm/lib/libSoundTouch.a" ]; then
    cd ../
    
    if [ ! -d "soundtouch" ]; then
        git clone https://gitlab.com/soundtouch/soundtouch.git
    fi
    
    cd soundtouch
    mkdir -p build-wasm
    cd build-wasm
    
    emcmake cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DENABLE_OPENMP=OFF
    
    emmake make -j4
    
    # Copy libraries to our directory
    cp libSoundTouch.a ../../soundtouch-wasm/lib/
    cp -r ../SoundTouch/*.h ../../soundtouch-wasm/include/
    
    cd ../../src
fi

echo "Building VoiceChanger for WebAssembly..."

# Build with qmake
qmake voiceChanger-wasm.pro CONFIG+=wasm
make clean
make -j4

echo "WebAssembly build completed!"
echo "Output files:"
echo "  - VoiceChanger.html (main application)"
echo "  - VoiceChanger.js (JavaScript wrapper)"
echo "  - VoiceChanger.wasm (WebAssembly module)"

# Create a simple HTML wrapper for testing
cat > index.html << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>VoiceChanger WebAssembly</title>
    <meta charset="utf-8">
    <style>
        body { margin: 0; padding: 20px; font-family: Arial, sans-serif; }
        #container { width: 100%; height: 600px; }
        .loading { text-align: center; margin-top: 200px; }
    </style>
</head>
<body>
    <h1>VoiceChanger WebAssembly</h1>
    <div id="container" class="loading">
        <p>Loading VoiceChanger...</p>
    </div>
    <script src="VoiceChanger.js"></script>
    <script>
        Module = {
            preRun: [],
            postRun: [],
            print: (text) => console.log(text),
            printErr: (text) => console.error(text),
            canvas: document.getElementById('container')
        };
        
        VoiceChangerModule().then(module => {
            console.log('VoiceChanger loaded successfully!');
            document.querySelector('.loading').style.display = 'none';
        });
    </script>
</body>
</html>
EOF

echo "Created index.html for testing"
echo "To test: python3 -m http.server 8000"
echo "Then open: http://localhost:8000"
