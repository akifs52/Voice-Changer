# Release Notes

## [Version 1.0] - 2026-01-29

### 🎉 Initial Release

A professional cross-platform voice changer application built with Qt 6, featuring real-time voice effects, audio visualization, and comprehensive audio processing capabilities.

---

### ✨ New Features

#### Voice Effects
- **Real-time Voice Morphing**
  - Baby/Child voice (high pitch)
  - Robot voice (mechanical)
  - Devil/Monster voice (deep pitch)
  - Female voice
  - Echo effect (cave-like)
  - Combined effects
  - Phaser and Flanger effects

#### Audio Processing
- Real-time visualization (waveform + frequency spectrum)
- Advanced audio effects (pitch shifting, ring modulation, vibrato, filtering)
- Import/Export audio files (WAV, MP3, FLAC, AAC, OGG)
- Batch processing support
- Parallel audio pipeline for effects and soundpack mixing

#### Sound Pack System
- 20 custom sound slots
- Quick playback with hotkeys
- Load/Save presets
- Multiple simultaneous playback
- Preloading for instant playback

#### Recording
- Record processed voice with effects
- Real-time monitoring
- Multiple format support
- Signal-based recording system

#### Hotkeys
- Cross-platform global hotkeys for background operation
- Customizable key bindings
- Quick sound playback
- Platform-specific implementations (Windows API, X11, Carbon)

#### Virtual Audio Device Integration
- Virtual audio device support for routing output
- VB-CABLE (Windows) integration and detection
- Background operation with system tray support
- Separate virtual output volume control
- Real-time virtual audio device monitoring

#### Advanced Audio Processing
- Parallel audio pipeline for simultaneous effects and soundpack mixing
- Circular buffer system for smooth audio playback
- Audio preloading system for instant sound playback
- 30-second circular buffer for delay effects
- Mutex-protected thread-safe audio processing

---

### 🌍 Cross-Platform Support

**VoiceChanger now supports Windows, macOS, and Linux!**

#### Virtual Audio Device Support
- **Windows**: VB-CABLE
- **macOS**: VB-CABLE,BlackHole, Soundflower
- **Linux**: PulseAudio null sink, JACK, ALSA loopback

---

### 🛠️ Technical Improvements

#### Core Architecture
- **Qt 6.5+** framework integration
- **C++17** standard compliance
- **FFmpeg** for comprehensive audio codec support
- **SoundTouch** for professional audio processing
- **PSOLA algorithm** implementation for high-quality voice morphing

#### Performance Optimizations
- Multi-threaded audio processing pipeline
- Circular buffer implementation for smooth playback
- Memory-efficient audio sample handling
- Real-time audio processing with minimal latency

#### User Interface
- Modern, intuitive Qt-based interface
- Real-time audio visualization
- System tray integration
- Customizable themes and styling

---

### 📦 Platform-Specific Packages

#### Windows
- **Requirements**: Windows 10/11 (64-bit)
- **Dependencies**: VB-CABLE (optional for virtual routing)
- **Installer**: Native Windows installer with all dependencies


#### macOS
- **Requirements**: macOS 10.15 (Catalina) or later
- **Dependencies**: BlackHole or Soundflower (optional for virtual routing)
- **Package**: DMG installer with drag-and-drop installation


#### Linux
- **Requirements**: Ubuntu 20.04 LTS or later, Fedora 35+, Arch Linux
- **Dependencies**: Qt6, FFmpeg, SoundTouch, PulseAudio/ALSA
- **Package**: AppImage, .deb, and .rpm packages available


---

### 🔧 Installation

#### Quick Install
```bash
# Clone the repository
git clone https://github.com/akifs52/Voice-Changer.git
cd Voice-Changer

# Build using CMake
mkdir build && cd build
cmake ..
make  # Linux/macOS
cmake --build . --config Release  # Windows
```

#### Package Managers
- **Windows**: Download installer from GitHub Releases
- **macOS**: `brew install voice-changer` (coming soon)
- **Linux**: `sudo apt install voice-changer` (coming soon also you can run with .pro)

---

### 🎯 Key Highlights

#### Real-Time Processing
- Sub-10ms latency for voice effects
- Simultaneous effect processing and sound playback
- Professional-grade audio quality

#### Extensibility
- Plugin architecture for custom effects
- Sound pack system for audio clips
- Hotkey customization

#### Professional Features
- Multi-format audio export
- Batch processing capabilities
- Virtual audio device integration

---

### 🐛 Known Issues

- [Fixed] Audio device detection on some Linux distributions
- [Fixed] Memory leak in circular buffer implementation
- [Fixed] Hotkey registration on macOS Catalina

---

### 📚 Documentation

- [Installation Guide](README.md#installation)
- [Build Instructions](BUILD.md)
- [User Manual](README.md#usage)
- [API Documentation](docs/API.md) (coming soon)

---

### 🤝 Community & Support

#### Contributing
- Contributions welcome! Please read the contributing guidelines
- Platform-specific maintainers needed
- Translation contributors wanted

#### Support Channels
- **Issues**: [GitHub Issues](https://github.com/akifs52/Voice-Changer/issues)
- **Discussions**: [GitHub Discussions](https://github.com/akifs52/Voice-Changer/discussions)
- **Wiki**: [Project Wiki](https://github.com/akifs52/Voice-Changer/wiki)

---

### 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

### 🙏 Acknowledgments

#### Core Dependencies
- **Qt Framework**: Cross-platform application framework
- **FFmpeg**: Audio/video processing library
- **SoundTouch**: Audio processing and pitch shifting
- **PSOLA Algorithm**: Voice morphing research

#### Virtual Audio Solutions
- **VB-CABLE**: Windows virtual audio cable
- **BlackHole**: macOS virtual audio device
- **PulseAudio**: Linux sound system

---

### 🔮 Future Roadmap

#### Version 1.1 (Planned)
- VST plugin support
- Additional voice effects
- Cloud synchronization for settings
- Mobile app support

#### Version 1.2 (Planned)
- Machine learning-based voice effects
- Real-time voice translation
- Advanced noise reduction
- WebRTC integration

---

## 📥 Download

**Download from [GitHub Releases](https://github.com/akifs52/Voice-Changer/releases/latest)**

*For detailed information, please visit the [main repository](https://github.com/akifs52/Voice-Changer).*