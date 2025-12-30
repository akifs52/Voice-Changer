#ifndef VOICEEFFECTS_H
#define VOICEEFFECTS_H

#include <QObject>
#include <cmath>
#include <vector>
#include <cstring>
#include <SoundTouchDLL.h>

class VoiceEffects : public QObject
{
    Q_OBJECT

public:
    explicit VoiceEffects(QObject *parent = nullptr);
    ~VoiceEffects();

    // Main processing functions
    void processBanana(float* input, float* output, int bufferSize, float sampleRate);
    void processDevil(float* input, float* output, int bufferSize, float sampleRate);
    void processFemale(float* input, float* output, int bufferSize, float sampleRate);
    void processMilitary(float* input, float* output, int bufferSize, float sampleRate);
    void processEko(float* input, float* output, int bufferSize, float sampleRate);

    // Utility functions
    void resetEffects();

private:
    // SoundTouch handles for different effects
    HANDLE bananaProcessor;
    HANDLE devilProcessor;
    HANDLE femaleProcessor;
    HANDLE militaryProcessor;
    HANDLE ekoProcessor;

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

    // Private helper functions
    void initializeSoundTouch();
    void initializeFilters();
    void initializeChorus();
    void initializeCompressor();
    void initializeEcho();

    // DSP processing functions
    void applyLowPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate);
    void applyHighPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate);
    void applyBandPass(float* input, float* output, int bufferSize, float lowFreq, float highFreq, float sampleRate);
    void applyChorus(float* input, float* output, int bufferSize, float sampleRate);
    void applyDistortion(float* input, float* output, int bufferSize);
    void applyCompressor(float* input, float* output, int bufferSize, float sampleRate);
    void applyEcho(float* input, float* output, int bufferSize, float sampleRate);
    
    // Utility functions
    void normalizeGain(float* buffer, int bufferSize);
    void protectClipping(float* buffer, int bufferSize);
    float tanhSoftClip(float x);
    void designLowPass(float cutoff, float sampleRate, FilterState& filter);
    void designHighPass(float cutoff, float sampleRate, FilterState& filter);
    void designBandPass(float lowFreq, float highFreq, float sampleRate, FilterState& filter);
};

#endif // VOICEEFFECTS_H
