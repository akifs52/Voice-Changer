# Contributing to Voice Changer

Thank you for your interest in contributing to Voice Changer! This document provides guidelines for contributors.

## Getting Started

### Prerequisites
- Qt 6.5.0 or later
- FFmpeg development libraries
- SoundTouch development libraries
- C++17 compatible compiler
- Git

### Setting Up Development Environment

1. Fork the repository
2. Clone your fork:
   ```bash
   git clone https://github.com/YOUR_USERNAME/Voice-Changer.git
   cd Voice-Changer
   ```

3. Add the original repository as upstream:
   ```bash
   git remote add upstream https://github.com/akifs52/Voice-Changer.git
   ```

4. Install dependencies for your platform (see README.md)

5. Build the project:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## Development Guidelines

### Code Style
- Use C++17 features appropriately
- Follow Qt coding conventions
- Use meaningful variable and function names
- Add comments for complex algorithms
- Keep functions focused and small

### Commit Messages
- Use clear, descriptive commit messages
- Start with a capital letter
- Use present tense ("Add feature" not "Added feature")
- Reference issue numbers when applicable

Example:
```
Add real-time pitch shifting effect

Implements PSOLA algorithm for smooth pitch shifting without
affecting playback speed. Fixes #123.
```

### Branch Naming
- Use descriptive branch names
- Format: `feature/description`, `bugfix/description`, `hotfix/description`

Examples:
- `feature/virtual-audio-support`
- `bugfix/crash-on-startup`
- `hotfix/memory-leak`

## Pull Request Process

1. Create a new branch for your feature
2. Make your changes
3. Test thoroughly on your platform
4. Update documentation if needed
5. Submit a pull request

### Pull Request Requirements
- Include tests for new features
- Update README.md if adding new functionality
- Ensure code compiles without warnings
- Test on multiple platforms if possible
- Link to related issues

### Testing
- Test voice effects functionality
- Test audio device detection
- Test recording and playback
- Test hotkey functionality
- Test virtual audio device integration

## Platform-Specific Contributions

### Windows
- VB-CABLE integration
- Windows API optimizations
- Installer improvements

### macOS
- BlackHole/Soundflower integration
- CoreAudio optimizations
- macOS-specific UI improvements

### Linux
- PulseAudio/JACK/ALSA support
- Distribution-specific packaging
- Linux hotkey improvements

## Bug Reports

When reporting bugs, please include:
- Operating system and version
- Qt version
- Audio devices being used
- Steps to reproduce
- Expected vs actual behavior
- Any error messages

## Feature Requests

Feature requests should include:
- Clear description of the feature
- Use case scenarios
- Implementation suggestions (if any)

## Questions

If you have questions about contributing:
- Check existing issues and discussions
- Create a new issue with the "question" label
- Join our community discussions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Code of Conduct

Be respectful and constructive in all interactions. We welcome contributors from all backgrounds and experience levels.

Thank you for contributing to Voice Changer!
