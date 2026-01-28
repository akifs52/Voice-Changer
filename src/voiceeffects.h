#ifndef VOICEEFFECTS_H
#define VOICEEFFECTS_H

#include <QObject>
#include <cmath>
#include <vector>
#include <cstring>
#include <cstdint>

// Cross-platform handle type
#ifdef Q_OS_WIN
#include <windows.h>
typedef HANDLE SoundTouchHandle;
#include <SoundTouchDLL.h>
#elif defined(Q_OS_LINUX)
#include <SoundTouch.h>
typedef void* SoundTouchHandle;
#else
typedef void* SoundTouchHandle;
#endif

class VoiceEffects : public QObject
{
    Q_OBJECT

public:
    explicit VoiceEffects(QObject *parent = nullptr);
    ~VoiceEffects();

    // Main processing functions
    void processRobot(float* input, float* output, int bufferSize, float sampleRate);
    void processBanana(float* input, float* output, int bufferSize, float sampleRate);
    void processDevil(float* input, float* output, int bufferSize, float sampleRate);
    void processFemale(float* input, float* output, int bufferSize, float sampleRate);
    void processMilitary(float* input, float* output, int bufferSize, float sampleRate);
    void processEko(float* input, float* output, int bufferSize, float sampleRate);
    void processPhase(float* input, float* output, int bufferSize, float sampleRate);
    void processFlanger(float* input, float* output, int bufferSize, float sampleRate);

    // Utility functions
    void resetEffects();

private:
    // Cross-platform SoundTouch helper functions
    void setSampleRate(SoundTouchHandle handle, uint rate);
    void setChannels(SoundTouchHandle handle, uint channels);
    void setPitchSemiTones(SoundTouchHandle handle, float pitch);
    void setRate(SoundTouchHandle handle, float rate);
    void setTempo(SoundTouchHandle handle, float tempo);
    void setSetting(SoundTouchHandle handle, int settingId, int settingValue);
    void putSamples(SoundTouchHandle handle, const float* samples, uint numSamples);
    uint receiveSamples(SoundTouchHandle handle, float* samples, uint maxSamples);
    void flush(SoundTouchHandle handle);
    void clear(SoundTouchHandle handle);

    // SoundTouch handles for different effects
    SoundTouchHandle robotProcessor;
    SoundTouchHandle bananaProcessor;
    SoundTouchHandle devilProcessor;
    SoundTouchHandle femaleProcessor;
    SoundTouchHandle militaryProcessor;
    SoundTouchHandle ekoProcessor;
    SoundTouchHandle phaserProcessor;
    SoundTouchHandle flangerProcessor;

    // Filter parameters
    struct FilterState {
        float x1, x2, y1, y2;
        float b0, b1, b2, a0, a1, a2;
    } lowPass, highPass, bandPass;

    // Chorus parameters
    struct ChorusState {
        std::vector<float> delayBuffer;
        int delayBufferSize;
        int writeIndex;
        float lfoPhase;
        float lfoFreq;
        float delayDepth;
    } chorus;

    // Distortion parameters
    struct DistortionState {
        float drive;
        float mix;
    } distortion;

    // Compressor parameters
    struct CompressorState {
        float threshold;
        float ratio;
        float attackTime;
        float releaseTime;
        float envelope;
        float gainReduction;
    } compressor;

    // Echo parameters
    struct EchoState {
        std::vector<float> delayBuffer;
        int delayBufferSize;
        int writeIndex;
        float feedback;
        float wetLevel;
    } echo;

    // Pitch correction parameters for autotune
    struct PitchCorrectionState {
        float targetFreq;
        float currentFreq;
        float correctionSpeed;
        float windowSize;
        std::vector<float> analysisBuffer;
        int analysisIndex;
    } pitchCorrection;

    // Radio/static parameters for Combine effect
    struct RadioStaticState {
        std::vector<float> noiseBuffer;
        int noiseIndex;
        float staticLevel;
        float crackleProbability;
        float lfoPhase;
        float carrierFreq;
    } radioStatic;

    // Gate parameters for robot effect
    struct GateState {
        float threshold;
        float attackTime;
        float releaseTime;
        float envelope;
        float sampleRate;
        float attackCoeff;
        float releaseCoeff;
    } gate;

    // Phaser parameters - Enhanced MXR Phase 90 style
    struct PhaserState {
        std::vector<float> z1;     // All-pass filtre durum değişkenleri (4 stage için)
        float lfoPhase;            // LFO fazı
        float rate;                // LFO hızı (Hz)
        float depth;               // Derinlik (0-1)
        float feedback;            // Geri besleme (-1 ile 1 arası)
        float mix;                 // Dry/Wet karışımı
        int stages;                // All-pass filtre stage sayısı (genellikle 4 veya 6)
        float minFreq;             // Minimum all-pass merkez frekansı
        float maxFreq;             // Maksimum all-pass merkez frekansı
    } phaser;

    // Flanger parameters
    struct FlangerState {
        std::vector<float> delayBuffer;
        int delayBufferSize;
        int delayIndex;
        float lfoPhase;
        float lfoFreq;
        float feedback;
        float depth;
        float delayTime;
    } flanger;

    // Private helper functions
    void initializeSoundTouch();
    void initializeFilters();
    void initializeChorus();
    void initializeCompressor();
    void initializeEcho();
    void initializePitchCorrection();
    void initializeRadioStatic();
    void initializeGate();
    void initializePhaser();
    void initializeFlanger();

    // DSP processing functions
    void applyLowPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate);
    void applyHighPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate);
    void applyBandPass(float* input, float* output, int bufferSize, float lowFreq, float highFreq, float sampleRate);
    void applyChorus(float* input, float* output, int bufferSize, float sampleRate);
    void applyDistortion(float* input, float* output, int bufferSize);
    void applyCompressor(float* input, float* output, int bufferSize, float sampleRate);
    void applyEcho(float* input, float* output, int bufferSize, float sampleRate);
    void applyPitchCorrection(float* input, float* output, int bufferSize, float sampleRate);
    void applyRadioStatic(float* input, float* output, int bufferSize, float sampleRate);
    void applyGate(float* input, float* output, int bufferSize, float sampleRate);
    void applyPhase(float* input, float* output, int bufferSize, float sampleRate);
    void applyFlanger(float* input, float* output, int bufferSize, float sampleRate);
    
    // Utility functions
    void normalizeGain(float* buffer, int bufferSize);
    void protectClipping(float* buffer, int bufferSize);
    float tanhSoftClip(float x);
    void designLowPass(float cutoff, float sampleRate, FilterState& filter);
    void designHighPass(float cutoff, float sampleRate, FilterState& filter);
    void designBandPass(float lowFreq, float highFreq, float sampleRate, FilterState& filter);
};

#endif // VOICEEFFECTS_H
