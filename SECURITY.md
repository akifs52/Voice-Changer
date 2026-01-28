# Security Policy

## Supported Versions

| Version | Supported |
|---------|-----------|
| 1.0.x   | ✅        |
| < 1.0   | ❌        |

## Reporting a Vulnerability

If you discover a security vulnerability in Voice Changer, please report it privately before disclosing it publicly.

### How to Report

1. **Email**: Send an email to security@voicechanger.app with details about the vulnerability
2. **GitHub Security**: Use GitHub's private vulnerability reporting feature
3. **Include in your report**:
   - Type of vulnerability (e.g., buffer overflow, code injection, etc.)
   - Steps to reproduce the vulnerability
   - Potential impact of the vulnerability
   - Any proof-of-concept code or screenshots

### Response Time

- **Critical**: Within 24 hours
- **High**: Within 48 hours  
- **Medium**: Within 72 hours
- **Low**: Within 1 week

### Security Updates

When a security vulnerability is reported:
1. We will acknowledge receipt within 24 hours
2. We will investigate and validate the vulnerability
3. We will develop a patch
4. We will release a security update
5. We will credit the reporter (if desired)

## Security Best Practices

### For Users
- Download Voice Changer only from official sources
- Keep your application updated
- Use antivirus software
- Be cautious with audio files from untrusted sources

### For Developers
- Validate all audio input
- Use secure audio processing libraries
- Follow secure coding practices
- Regular security audits

## Security Features

Voice Changer includes several security features:
- Input validation for audio files
- Safe memory management
- Secure audio processing pipelines
- No network connectivity by default

## Known Security Considerations

### Audio File Processing
- Voice Changer processes user-provided audio files
- Malformed audio files could potentially cause crashes
- We use FFmpeg's built-in security features for audio processing

### Virtual Audio Devices
- Integration with third-party virtual audio devices
- Users should only install virtual audio devices from trusted sources

### Hotkey System
- Global hotkeys use platform-specific APIs
- Implemented with security considerations in mind

## Responsible Disclosure

We believe in responsible disclosure and will work with security researchers to:
- Acknowledge reports promptly
- Provide regular updates on our progress
- Coordinate disclosure timelines
- Credit researchers for their findings

## Security Contacts

- **Security Email**: security@voicechanger.app
- **GitHub Security**: https://github.com/akifs52/Voice-Changer/security

Thank you for helping keep Voice Changer secure!
