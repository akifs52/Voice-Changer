


# Voice Changer

<img width="1351" height="1007" alt="image" src="https://github.com/user-attachments/assets/c94ea3dd-adff-4574-88c1-2c2e43a3efee" />


A professional cross-platform voice changer application built with Qt 6, featuring real-time voice effects, audio visualization, and comprehensive audio processing capabilities.

## Features

### Voice Effects
- **Real-time Voice Morphing**
  - Baby/Child voice (high pitch)
  - Robot voice (mechanical)
  - Devil/Monster voice (deep pitch)
  - Female voice
  - Echo effect (cave-like)
  - Combined effects
  - Phaser and Flanger effects

### Audio Processing
- Real-time visualization (waveform + frequency spectrum)
- Advanced audio effects (pitch shifting, ring modulation, vibrato, filtering)
- Import/Export audio files (WAV, MP3, FLAC, AAC, OGG)
- Batch processing support
- Parallel audio pipeline for effects and soundpack mixing

### Sound Pack System
- 20 custom sound slots
- Quick playback with hotkeys
- Load/Save presets
- Multiple simultaneous playback
- Preloading for instant playback

### Recording
- Record processed voice with effects
- Real-time monitoring
- Multiple format support
- Signal-based recording system

### Hotkeys
- Cross-platform global hotkeys for background operation
- Customizable key bindings
- Quick sound playback
- Platform-specific implementations (Windows API, X11, Carbon)

### Virtual Audio Device Integration
- Virtual audio device support for routing output
- VB-CABLE (Windows) integration and detection
- Background operation with system tray support
- Separate virtual output volume control
- Real-time virtual audio device monitoring

### Advanced Audio Processing
- Parallel audio pipeline for simultaneous effects and soundpack mixing
- Circular buffer system for smooth audio playback
- Audio preloading system for instant sound playback
- 30-second circular buffer for delay effects
- Mutex-protected thread-safe audio processing

## Cross-Platform Support

**VoiceChanger now supports Windows, macOS, and Linux!**

### Virtual Audio Device Support
- **Windows**: VB-CABLE
- **macOS**: BlackHole, Soundflower
- **Linux**: PulseAudio null sink, JACK, ALSA loopback

## Installation

### Clone the Repository
```bash
git clone https://github.com/akifs52/Voice-Changer.git
cd Voice-Changer
```

### System Requirements

#### Common Requirements
- Qt 6.5.0 or later
- FFmpeg development libraries
- SoundTouch development libraries

#### Platform-Specific Requirements
- **Windows**: Windows 10/11 (64-bit), Visual Studio 2019+ or MinGW-w64
- **macOS**: macOS 10.15 (Catalina) or later, Xcode 12.0+
- **Linux**: GCC 9.0+ or Clang 10.0+, PulseAudio/ALSA

### Quick Start

#### Windows
1. Install VB-CABLE from https://vb-audio.com/Cable/
2. Download and run the installer
3. Restart computer
4. Launch VoiceChanger

#### macOS
```bash
# Install dependencies with Homebrew
brew install qt6 ffmpeg soundtouch blackhole

# Clone and build
git clone https://github.com/akifs52/Voice-Changer.git
cd Voice-Changer
mkdir build && cd build
cmake ..
make
./VoiceChanger
```

#### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential qt6-base-dev qt6-multimedia-dev
sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev
sudo apt install libsoundtouch-dev libpulse-dev

# Clone and build
git clone https://github.com/akifs52/Voice-Changer.git
cd Voice-Changer
mkdir build && cd build
cmake ..
make
./VoiceChanger
```

### Virtual Audio Device Setup

#### Windows
- Download VB-CABLE A+B (free) from https://vb-audio.com/Cable/
- Install as Administrator
- Restart computer

#### macOS
- **Option 1 (Recommended)**: Install BlackHole
  ```bash
  brew install blackhole
  ```
- **Option 2**: Install Soundflower from https://github.com/mattingalls/Soundflower

#### Linux
- **Option 1**: PulseAudio null sink
  ```bash
  pactl load-module module-null-sink sink_name=virtual
  pavucontrol
  ```
- **Option 2**: JACK Audio Connection Kit
  ```bash
  sudo apt install jackd2 qjackctl
  ```
- **Option 3**: ALSA loopback
  ```bash
  sudo modprobe snd-aloop
  ```

## Build Instructions

```bash
git clone https://github.com/akifs52/Voice-Changer.git
cd Voice-Changer
```

For detailed build instructions, see [BUILD.md](BUILD.md).

### Using Qt Creator (All Platforms)
1. Open Qt Creator
2. File -> Open File or Project -> Select `voiceChanger.pro`
3. Configure the kit for your platform
4. Build -> Build All (Ctrl+B)
5. Run (Ctrl+R)

### Using CMake (Alternative)
```bash
mkdir build && cd build
cmake ..
make  # Linux/macOS
# or
cmake --build . --config Release  # Windows
```

## Usage

### Basic Setup

1. **Select Audio Devices**
   - Choose your input microphone from the dropdown
   - Choose your output speakers/headphones
   - (Optional) Select virtual audio device for routing

   **⚠️ IMPORTANT: YOU HAVE TO CHOOSE DEFAULT OUTPUT TO VB-CABLE FOR YOUR SOUNDS REACH OTHERS OR CHOOSE IN YOUR GAME OR CHAT APP**

2. **Test Your Setup**
   - Click "Test Device" to hear your voice through speakers
   - Adjust input/output volume sliders as needed

### Using Voice Effects

1. Click on any effect button (Robot, Baby Voice, etc.)
2. The button will turn green when active
3. Speak to hear the transformed voice
4. Click the same button again (now labeled "Stop") to deactivate

### Using Sound Packs

1. **Assign Sound Files**
   - Click "s1" (save) button next to a sound slot
   - Select an audio file (WAV, MP3, etc.)

2. **Play Sounds**
   - Click on any sound button to play
   - Multiple sounds can play simultaneously

3. **Set Hotkeys**
   - Click on the hotkey dropdown next to a sound
   - Press the desired key combination

4. **Load/Save Presets**
   - Click "l1" (load) to load a saved preset
   - Click "s1" (save) to save your current setup

### Recording

1. Click "Start Record" button
2. All processed audio will be recorded
3. Click "Stop Record" to stop
4. Recording is automatically saved

## System Requirements

### Supported Platforms
- **Windows**: Windows 10/11 (64-bit)
- **macOS**: macOS 10.15 (Catalina) or later
- **Linux**: Ubuntu 20.04 LTS or later, Fedora 35+, Arch Linux

### Dependencies
- Qt 6.5.0 or later
- FFmpeg development libraries
- SoundTouch development libraries
- Platform-specific audio frameworks (see BUILD.md)

## Repository

GitHub: https://github.com/akifs52/Voice-Changer

## License

This project is licensed under the MIT License.

## Credits

- Qt Framework: https://www.qt.io/
- FFmpeg: https://ffmpeg.org/
- SoundTouch: http://www.surina.net/soundtouch/
- PSOLA Algorithm: Various research papers
- Virtual Audio Solutions:
  - VB-CABLE: https://vb-audio.com/ (Windows)
  - BlackHole: https://github.com/ExistentialAudio/BlackHole (macOS)
  - PulseAudio: https://www.freedesktop.org/wiki/Software/PulseAudio/ (Linux)

## Contributing

Contributions are welcome! Please read the contributing guidelines and submit pull requests.

### Platform-Specific Contributions
- **Windows**: VB-CABLE integration, Windows API improvements
- **macOS**: BlackHole/Soundflower integration, macOS-specific optimizations
- **Linux**: PulseAudio/JACK/ALSA support, distribution-specific packaging

## Support

For platform-specific issues:
- **Windows**: Check VB-CABLE installation and Windows Audio service
- **macOS**: Check microphone permissions and CoreAudio settings
- **Linux**: Check PulseAudio service and user permissions

For general issues, please check the [BUILD.md](BUILD.md) file and create an issue on GitHub.
