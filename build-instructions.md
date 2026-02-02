# Build Instructions for VoiceChanger

## Prerequisites

### For All Platforms
- Qt 6.5+ with Qt Multimedia
- CMake 3.20+ (if using CMake)
- C++17 compatible compiler

### For WebAssembly
- Emscripten SDK
  ```bash
  git clone https://github.com/emscripten-core/emsdk.git
  cd emsdk
  emsdk install latest
  emsdk activate latest
  source ./emsdk_env.sh
  ```

## Desktop Build

### Using qmake
```bash
# Navigate to src directory
cd src

# Build for current platform
qmake voiceChanger.pro
make -j4  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

### Using CMake
```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build . --config Release
```

## WebAssembly Build

### Prepare Libraries

#### FFmpeg for WebAssembly
```bash
# Clone and build FFmpeg
git clone https://github.com/FFmpeg/FFmpeg.git
cd FFmpeg
emconfigure ./configure \
    --prefix=../ffmpeg-wasm-lib \
    --disable-programs \
    --disable-doc \
    --disable-shared \
    --enable-static \
    --target-os=none \
    --arch=x86_32 \
    --cpu=generic \
    --enable-avcodec \
    --enable-avformat \
    --enable-avutil \
    --enable-swresample \
    --enable-swscale
emmake make -j4
emmake make install
```

#### SoundTouch for WebAssembly
```bash
# Clone and build SoundTouch
git clone https://gitlab.com/soundtouch/soundtouch.git
cd soundtouch
mkdir build-wasm && cd build-wasm
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF
emmake make -j4

# Copy libraries
mkdir -p ../../soundtouch-wasm/lib
mkdir -p ../../soundtouch-wasm/include
cp libSoundTouch.a ../../soundtouch-wasm/lib/
cp -r ../SoundTouch/*.h ../../soundtouch-wasm/include/
```

### Build WebAssembly Version

#### Using qmake
```bash
cd src

# Configure for WebAssembly
qmake CONFIG+=wasm voiceChanger.pro
make clean
make -j4
```

#### Using CMake
```bash
mkdir build-wasm && cd build-wasm

# Configure with Emscripten
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j4
```

### Run WebAssembly Version
```bash
# Start local server
python3 -m http.server 8000

# Open in browser
# http://localhost:8000/VoiceChanger.html
```

## Platform-Specific Notes

### Windows
- Install VB-CABLE for virtual audio routing
- FFmpeg and SoundTouch paths in .pro file may need adjustment

### macOS
- Install BlackHole or Soundflower for virtual audio routing
- Use Homebrew for dependencies: `brew install ffmpeg soundtouch`

### Linux
- Install dependencies: `sudo apt install qt6-base-dev qt6-multimedia-dev ffmpeg libsoundtouch-dev`
- For virtual audio: `sudo apt install pulseaudio-utils`

### WebAssembly
- Memory limits are configured (64MB initial, 512MB max)
- Threading support enabled (4 threads)
- File system access through IDBFS
- Audio processing optimized for Web Audio API

## Build Options

### Debug Build
```bash
# qmake
qmake CONFIG+=debug voiceChanger.pro
make

# CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

### Release Build
```bash
# qmake
qmake CONFIG+=release voiceChanger.pro
make

# CMake
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
```

### WebAssembly Specific Options
```bash
# Enable debugging
emcc -s ASSERTIONS=1 -s DEMANGLE_SUPPORT=1

# Optimize for size
emcc -Os -flto

# Enable profiling
emcc -s PROFILING=1
```

## Troubleshooting

### Common Issues

1. **FFmpeg not found**
   - Check library paths in .pro or CMakeLists.txt
   - Ensure FFmpeg is built for target platform

2. **SoundTouch linking errors**
   - Verify SoundTouch is built with same compiler
   - Check library paths and names

3. **WebAssembly memory issues**
   - Increase memory limits in linker flags
   - Check for memory leaks in audio processing

4. **Audio device not found**
   - Install virtual audio cable (VB-CABLE, BlackHole)
   - Check system audio permissions

5. **Qt Multimedia not available**
   - Install Qt Multimedia package
   - Check Qt version compatibility

### Performance Optimization

- Use Release builds for production
- Enable compiler optimizations (-O3)
- Consider static linking for WebAssembly
- Profile with platform-specific tools

## Deployment

### Desktop
- Create installer packages
- Bundle dependencies
- Test on target systems

### WebAssembly
- Optimize file sizes
- Test in different browsers
- Consider CDN hosting
- Implement progressive loading
