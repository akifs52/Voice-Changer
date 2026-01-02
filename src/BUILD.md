# Cross-Platform Build Instructions

## Requirements

### Common Requirements
- Qt 6.5.0 or later
- CMake 3.16 or later (optional, for alternative build system)
- FFmpeg development libraries
- SoundTouch development libraries

### Platform-Specific Requirements

#### Windows
- Visual Studio 2019 or later (with C++ support)
- MinGW-w64 64-bit compiler
- Windows 10/11

#### macOS
- Xcode 12.0 or later
- macOS 10.15 (Catalina) or later
- Homebrew (recommended for dependencies)

#### Linux
- GCC 9.0 or later, or Clang 10.0 or later
- PulseAudio development libraries
- ALSA development libraries

## Dependency Installation

### Windows

#### Using vcpkg (Recommended)
```bash
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install ffmpeg:x64-windows
.\vcpkg install soundtouch:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

#### Manual Installation
1. Download FFmpeg Windows builds from https://ffmpeg.org/download.html
2. Download SoundTouch DLL from http://www.surina.net/soundtouch/
3. Extract to appropriate folders and update paths in .pro file

### macOS

#### Using Homebrew (Recommended)
```bash
# Install dependencies
brew install ffmpeg
brew install soundtouch
brew install qt6
```

#### Manual Installation
```bash
# Install Qt
# Download from https://www.qt.io/download-qt-installer

# Install FFmpeg
brew install ffmpeg

# Install SoundTouch
brew install soundtouch
```

### Linux

#### Ubuntu/Debian
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential qt6-base-dev qt6-multimedia-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev
sudo apt install libsoundtouch-dev
sudo apt install libpulse-dev libasound2-dev
```

#### Fedora/CentOS/RHEL
```bash
# Install dependencies
sudo dnf install gcc-c++ qt6-qtbase-devel qt6-qtmultimedia-devel
sudo dnf install ffmpeg-devel
sudo dnf install soundtouch-devel
sudo dnf install pulseaudio-libs-devel alsa-lib-devel
```

#### Arch Linux
```bash
# Install dependencies
sudo pacman -S base-devel qt6-base qt6-multimedia
sudo pacman -S ffmpeg
sudo pacman -S soundtouch
sudo pacman -S pulseaudio alsa-lib
```

## Building the Application

### Using Qt Creator (All Platforms)
1. Open Qt Creator
2. File → Open Project → Select `voiceChanger.pro`
3. Configure build settings for your platform
4. Build → Build Project

### Using Command Line

#### Windows (MinGW)
```bash
# Configure Qt environment
# (Run from Qt Command Prompt)

# Build
qmake voiceChanger.pro
make
```

#### Windows (Visual Studio)
```bash
# Configure Qt environment
# (Run from Qt Command Prompt with VS tools)

# Build
qmake voiceChanger.pro -spec win32-msvc
nmake
```

#### macOS
```bash
# Build
qmake voiceChanger.pro
make
```

#### Linux
```bash
# Build
qmake voiceChanger.pro
make
```

## Virtual Audio Device Setup

### Windows
1. Download VB-CABLE from https://vb-audio.com/Cable/
2. Install as Administrator
3. Restart computer
4. Launch VoiceChanger

### macOS
1. Install BlackHole (Recommended):
   ```bash
   brew install blackhole
   ```
2. Or download from https://github.com/ExistentialAudio/BlackHole
3. Restart computer
4. Launch VoiceChanger

### Linux
#### Option 1: PulseAudio Null Sink
```bash
# Create null sink
pactl load-module module-null-sink sink_name=virtual

# Use pavucontrol to route audio
sudo apt install pavucontrol
pavucontrol
```

#### Option 2: JACK Audio Connection Kit
```bash
# Install JACK
sudo apt install jackd2 qjackctl

# Configure and start JACK
qjackctl
```

#### Option 3: ALSA Loopback
```bash
# Load loopback module
sudo modprobe snd-aloop

# Configure in ~/.asoundrc
# See online documentation for detailed configuration
```

## Troubleshooting

### Common Issues

#### Qt Not Found
```bash
# Make sure Qt is in your PATH
export PATH=/path/to/qt/bin:$PATH
export LD_LIBRARY_PATH=/path/to/qt/lib:$LD_LIBRARY_PATH  # Linux
export DYLD_LIBRARY_PATH=/path/to/qt/lib:$DYLD_LIBRARY_PATH  # macOS
```

#### FFmpeg Not Found
- Ensure FFmpeg development libraries are installed
- Check library paths in .pro file
- On Linux, try `pkg-config --cflags --libs libavcodec`

#### SoundTouch Not Found
- Ensure SoundTouch development libraries are installed
- Check library paths in .pro file
- On macOS with Homebrew: `brew --prefix soundtouch`

#### Audio Device Issues
- Ensure virtual audio device is properly installed
- Check system audio settings
- Restart audio system if needed

#### Permission Issues (Linux/macOS)
```bash
# Fix permissions for audio devices
sudo usermod -a -G audio $USER  # Linux
# Log out and log back in
```

### Platform-Specific Issues

#### Windows
- Run installer as Administrator
- Disable antivirus during installation
- Check Windows Audio service is running

#### macOS
- Grant microphone permissions in System Preferences
- Check security settings for downloaded apps
- Reset CoreAudio if needed: `sudo launchctl kickstart -k system/com.apple.audio.coreaudiod`

#### Linux
- Check PulseAudio is running: `pulseaudio --check`
- Ensure user is in audio group: `groups $USER`
- Restart PulseAudio: `pulseaudio -k && pulseaudio --start`

## Package Creation

### Windows
```bash
# Use windeployqt
windeployqt VoiceChanger.exe
# Create installer with Inno Setup or NSIS
```

### macOS
```bash
# Create app bundle
macdeployqt VoiceChanger.app
# Create DMG with hdiutil
```

### Linux
```bash
# Create AppImage
linuxdeploy --appdir AppDir --executable VoiceChanger --desktop-file VoiceChanger.desktop --output appimage
# Or create .deb package
dpkg-deb --build voicechanger-package
```

## Development Notes

- The application uses Qt Multimedia for cross-platform audio I/O
- Global hotkeys use platform-specific APIs (Windows API, X11, Carbon)
- Virtual audio device detection is cross-platform
- FFmpeg and SoundTouch are loaded dynamically based on platform

## Support

For issues specific to your platform, please check:
- Qt documentation: https://doc.qt.io/
- FFmpeg documentation: https://ffmpeg.org/documentation.html
- SoundTouch documentation: http://www.surina.net/soundtouch/
