# Voice Changer

<img width="1114" height="859" alt="image" src="https://github.com/user-attachments/assets/da9c07dd-af0b-411d-8623-a55f8e346b23" />

A professional voice changer application built with Qt 6, featuring real-time voice effects, audio visualization, and comprehensive audio processing capabilities.

## Features

### Voice Effects
- **Real-time Voice Morphing**
  - Baby/Child voice (high pitch)
  - Robot voice (mechanical)
  - Devil/Monster voice (deep pitch)
  - Female voice
  - Echo effect (cave-like)
  - Combined effects

### Audio Processing
- Real-time visualization (waveform + frequency spectrum)
- Advanced audio effects (pitch shifting, ring modulation, vibrato, filtering)
- Import/Export audio files (WAV, MP3, FLAC, AAC, OGG)
- Batch processing support

### Sound Pack System
- 20 custom sound slots
- Quick playback with hotkeys
- Load/Save presets
- Multiple simultaneous playback

### Recording
- Record processed voice
- Real-time monitoring
- Multiple format support

### Hotkeys
- Global hotkeys for background operation
- Customizable key bindings
- Quick sound playback

## Installation

### Prerequisites
- Windows 10/11 (64-bit)
- Qt 6.5.0 or later
- FFmpeg libraries
- VB-CABLE driver (optional, for virtual audio)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/Voice-Changer.git
cd Voice-Changer

# Install dependencies
pip install -r requirements.txt

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64"

# Build
cmake --build . --config Release

# Run
./VoiceChanger
```

### Using Qt Creator
1. Open Qt Creator
2. File -> Open File or Project -> Select `voiceChanger.pro`
3. Configure the kit
4. Build -> Build All (Ctrl+B)
5. Run (Ctrl+R)

## Usage

### Basic Setup

1. **Select Audio Devices**
   - Choose your input microphone from the dropdown
   - Choose your output speakers/headphones
   - Select VB-CABLE Input for virtual audio routing
   - !! YOU HAVE TO CHOOSE DEFAULT OUTPUT TO VB-CABLE FOR YOUR SOUNDS REACH OTHERS OR CHOOSE IN YOUR GAME OR CHAT APP

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

- **Windows**: Windows 10/11 (64-bit)
- **macOS**: macOS 11.0 (Big Sur) or later
- **Linux**: Ubuntu 20.04 LTS or later

## License

This project is licensed under the MIT License.

## Credits

- Qt Framework: https://www.qt.io/
- FFmpeg: https://ffmpeg.org/
- PSOLA Algorithm: Various research papers
- VB-CABLE: https://vb-audio.com/
