# Voice Changer Application

A professional, cross-platform voice changer application built with Qt 6, featuring real-time voice morphing, audio visualization, and comprehensive audio processing capabilities.

## Features

### Core Voice Effects
- **Real-time Voice Morphing**: Transform your voice instantly
  - Baby/Child voice (high pitch modulation)
  - Robot voice (mechanical modulation)
  - Devil/Monster voice (deep pitch modulation)
  - Female voice (feminization)
  - Echo effect (cave-like reverb)
  - Combined effects (custom voice blends)

### Audio Processing
- **Real-time Visualization**
  - Waveform display
  - Frequency spectrum analyzer
  - Combined view (waveform + spectrum)
  - Smooth 60 FPS rendering

- **Advanced Audio Effects**
  - Pitch shifting (PSOLA algorithm)
  - Ring modulation
  - Vibrato
  - High-pass/Low-pass filtering
  - Noise gate
  - Cave echo reverb

### Audio File Management
- **Import/Export**
  - Drag-and-drop file support
  - WAV, MP3, FLAC, AAC, OGG support
  - Batch processing
  - Format conversion

- **Audio Processing**
  - Resampling (various sample rates)
  - Format conversion
  - Effect application to files
  - Quality preservation

### Sound Pack System
- **20 Custom Sound Slots**
  - Assign custom audio files
  - Quick playback with hotkeys
  - Load/Save presets
  - Visual feedback during playback

### Recording
- **Audio Recording**
  - Record processed voice
  - Save to multiple formats
  - Real-time monitoring
  - Recording visualization

### Real-time Speech-to-Text
- **Chinese Speech Recognition**
  - Real-time transcription using Whisper
  - Multi-language support (Chinese, English, Japanese, etc.)
  - Automatic voice activity detection
  - Timestamps for each transcription segment
  - WebSocket API for integration

### Virtual Audio Device Support
- **VB-CABLE Integration**
  - Virtual audio input/output
  - Mix multiple audio sources
  - Stream to other applications
  - Auto-detection and setup

### Hotkeys
- **Global Hotkeys**
  - Background operation support
  - Customizable key bindings
  - Quick sound playback
  - Effect toggle

### Modern UI
- **Dark Theme**
  - Professional interface
  - Gradient effects
  - Smooth animations
  - High contrast display

## System Requirements

### Windows
- Windows 10/11 (64-bit)
- Qt 6.5.0 or later
- Visual Studio 2019 or later (MSVC)
- FFmpeg libraries (included)
- VB-CABLE driver (optional, for virtual audio)

### macOS
- macOS 11.0 (Big Sur) or later
- Qt 6.5.0 or later
- Xcode 12 or later
- FFmpeg libraries (via Homebrew)
- BlackHole driver (optional, for virtual audio)

### Linux
- Ubuntu 20.04 LTS or later / Fedora 34 or later
- Qt 6.5.0 or later
- GCC 9 or later / Clang 10 or later
- FFmpeg libraries (via package manager)
- PulseAudio/PipeWire

## Installation

### Prerequisites

#### Windows
1. Install Visual Studio 2019 or later with C++ development tools
2. Download and install Qt 6.5.0 (MSVC 2019 64-bit) from https://www.qt.io/download
3. Download FFmpeg libraries from https://ffmpeg.org/download.html
4. (Optional) Install VB-CABLE from https://vb-audio.com/Cable/

#### macOS
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install qt@6 ffmpeg
brew install --cask blackhole-2ch
```

#### Linux (Ubuntu/Debian)
```bash
# Update package list
sudo apt update

# Install dependencies
sudo apt install build-essential cmake qt6-base-dev qt6-multimedia-dev
sudo apt install libavcodec-dev libavformat-dev libswresample-dev libavutil-dev
sudo apt install pulseaudio pulseaudio-utils

# (Optional) Install virtual audio cable
sudo apt install pulseeffects
```

#### Linux (Fedora)
```bash
# Install dependencies
sudo dnf install gcc-c++ cmake qt6-qtbase-devel qt6-qtmultimedia-devel
sudo dnf install ffmpeg-devel
sudo dnf install pulseaudio pulseaudio-utils

# (Optional) Install virtual audio cable
sudo dnf install easyeffects
```

## Compilation

### Windows (Visual Studio + CMake)

```bash
# Clone or extract the project
cd D:\qt\voiceChanger

# Create build directory
mkdir build
cd build

# Configure with CMake (adjust Qt path as needed)
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"

# Build
cmake --build . --config Release

# Run
.\Release\VoiceChanger.exe
```

### Windows (qmake)

```bash
# Open Qt Creator
# File -> Open File or Project -> Select voiceChanger.pro
# Configure the kit (MSVC 2019 64-bit)
# Build -> Build All (Ctrl+B)
# Run (Ctrl+R)
```

### macOS

```bash
# Clone or extract the project
cd voiceChanger

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH="/usr/local/opt/qt@6"

# Build
cmake --build . --config Release

# Run
./VoiceChanger.app/Contents/MacOS/VoiceChanger
```

### Linux

```bash
# Clone or extract the project
cd voiceChanger

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH="/usr/lib/qt6"

# Build
cmake --build . -j$(nproc)

# Run
./VoiceChanger
```

## Usage

### Basic Setup

1. **Select Audio Devices**
   - Choose your input microphone from the dropdown
   - Choose your output speakers/headphones
   - (Optional) Select VB-CABLE Output for virtual audio routing

2. **Adjust Volumes**
   - Use input slider to adjust microphone level
   - Use output slider to adjust speaker level
   - Use virtual slider to adjust virtual audio level

3. **Test Your Setup**
   - Click "Test Device" to hear your voice through speakers
   - Speak to see the visualization and progress bar
   - Adjust levels as needed

### Using Voice Effects

1. **Activate an Effect**
   - Click on any effect button (Robot, Baby Voice, etc.)
   - The button will turn green when active
   - Speak to hear the transformed voice

2. **Deactivate**
   - Click the same button again (now labeled "Stop")
   - Your voice will return to normal

3. **Adjust Effects** (coming in v2.0)
   - Fine-tune pitch, speed, reverb, and more
   - Create custom effect presets

### Using Sound Packs

1. **Assign Sound Files**
   - Click "s1" (save) button next to a sound slot
   - Select an audio file (WAV, MP3, etc.)
   - The file is now assigned to that slot

2. **Play Sounds**
   - Click on any sound button to play
   - The button highlights during playback
   - Multiple sounds can play simultaneously

3. **Set Hotkeys**
   - Click on the hotkey dropdown next to a sound
   - Press the desired key combination
   - Use the hotkey to trigger the sound

4. **Load/Save Presets**
   - Click "l1" (load) to load a saved preset
   - Click "s1" (save) to save your current setup

### Recording

1. **Start Recording**
   - Click "Start Record" button
   - All processed audio will be recorded

2. **Stop Recording**
   - Click "Stop Record" button
   - The recording is automatically saved

3. **Recording Location**
   - Windows: `C:\Users\<YourName>\AppData\Roaming\voicechanger\recordings\`
   - macOS: `~/Library/Application Support/voicechanger/recordings/`
   - Linux: `~/.config/voicechanger/recordings/`

### File Import/Export

1. **Import Files**
   - Drag and drop audio files onto the application
   - Or use File -> Import Audio
   - Supported formats: WAV, MP3, FLAC, AAC, OGG

2. **Export Files**
   - Use File -> Export Audio
   - Choose the output format and location
   - Apply effects if desired

3. **Batch Processing**
   - File -> Batch Process
   - Select multiple files
   - Choose output format and effects
   - Process all files at once

### Virtual Audio Routing

1. **Setup Virtual Cable**
    - Install VB-CABLE driver (Windows)
    - Select "VB-CABLE Input" as your output device
    - Select "VB-CABLE Output" in applications like Discord, OBS, etc.

2. **Mix Audio**
    - Voice effects route to VB-CABLE automatically
    - Sound pack audio mixes with voice
    - Perfect for streaming and gaming

### Speech-to-Text

1. **Start the Backend Server**
    ```bash
    cd backend
    python server.py
    ```

2. **Enable Speech Recognition**
    - Connect to WebSocket at `ws://localhost:8765`
    - Send message: `{"type": "start_speech_recognition"}`
    - Start speaking in Chinese

3. **Receive Transcriptions**
    - The server broadcasts transcriptions to all clients
    - Format: `{"type": "transcription", "text": "...", "start": 0.0, "end": 1.5}`
    - Transcriptions appear in real-time as you speak

4. **Change Language**
    - Send message: `{"type": "set_speech_language", "language": "zh"}`
    - Supported: `zh` (Chinese), `en` (English), `ja` (Japanese), etc.

5. **Stop Recognition**
    - Send message: `{"type": "stop_speech_recognition"}`

For more details, see `backend/SPEECH_RECOGNITION.md`

## Troubleshooting

### Audio Not Working
- Check input/output device selection
- Verify system audio settings
- Try different sample rate (44.1kHz or 48kHz)
- Restart the application

### Effects Sound Distorted
- Reduce input volume
- Disable aggressive effects
- Check microphone quality
- Adjust noise gate settings

### High Latency
- Increase buffer size in settings
- Close other audio applications
- Use ASIO drivers (Windows) or Core Audio (macOS)
- Check system performance

### VB-CABLE Not Working
- Reinstall VB-CABLE driver
- Restart computer after installation
- Check VB-CABLE is selected as output
- Verify VB-CABLE is not muted

## Configuration Files

### Settings Location
- Windows: `%APPDATA%\voicechanger\settings.ini`
- macOS: `~/Library/Application Support/voicechanger/settings.ini`
- Linux: `~/.config/voicechanger/settings.ini`

### Hotkeys
- Hotkeys are saved in `hotkeys.ini` in the same directory
- Manually edit to create custom combinations

### Sound Packs
- Sound files are stored in `soundpack/` directory
- Supported formats: WAV, MP3, FLAC, OGG

## Building from Source

### Development Setup

```bash
# Clone the repository
git clone https://github.com/yourusername/voicechanger.git
cd voicechanger

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build . -j

# Run
./VoiceChanger
```

### Dependencies

- Qt 6.5+ (Core, Gui, Widgets, Multimedia)
- FFmpeg (libavcodec, libavformat, libswresample, libavutil)
- Platform-specific audio libraries

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly on multiple platforms
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Credits

- Qt Framework: https://www.qt.io/
- FFmpeg: https://ffmpeg.org/
- PSOLA Algorithm: Various research papers
- VB-CABLE: https://vb-audio.com/

## Changelog

### Version 1.0.0 (Current)
- Initial release
- Real-time voice effects
- Audio visualization
- Sound pack system
- Recording functionality
- Hotkey support
- File import/export
- Batch processing
- Virtual audio support

## Support

For issues, questions, or feature requests:
- GitHub Issues: https://github.com/yourusername/voicechanger/issues
- Email: support@voicechanger.example.com

## Acknowledgments

Thanks to all contributors and the open-source community for making this project possible!
