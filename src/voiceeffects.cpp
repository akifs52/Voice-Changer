#include "voiceeffects.h"
#include <algorithm>
#include <iostream>

VoiceEffects::VoiceEffects(QObject *parent)
    : QObject(parent)
{
    // Initialize SoundTouch processors
    initializeSoundTouch();
    
    // Initialize filter states
    initializeFilters();
    
    // Initialize chorus
    initializeChorus();
    
    // Initialize distortion
    distortion.drive = 2.0f;
    distortion.mix = 0.3f;
    
    // Initialize compressor
    compressor.threshold = 0.7f;
    compressor.ratio = 4.0f;
    compressor.attackTime = 0.003f;
    compressor.releaseTime = 0.1f;
    compressor.envelope = 0.0f;
    compressor.gainReduction = 0.0f;
    
    // Initialize echo
    initializeEcho();
}

VoiceEffects::~VoiceEffects()
{
    // Destroy SoundTouch processors
    if (bananaProcessor) soundtouch_destroyInstance(bananaProcessor);
    if (devilProcessor) soundtouch_destroyInstance(devilProcessor);
    if (femaleProcessor) soundtouch_destroyInstance(femaleProcessor);
    if (militaryProcessor) soundtouch_destroyInstance(militaryProcessor);
    if (ekoProcessor) soundtouch_destroyInstance(ekoProcessor);
}

void VoiceEffects::initializeSoundTouch()
{
    // Create SoundTouch instances for each effect
    bananaProcessor = soundtouch_createInstance();
    devilProcessor = soundtouch_createInstance();
    femaleProcessor = soundtouch_createInstance();
    militaryProcessor = soundtouch_createInstance();
    ekoProcessor = soundtouch_createInstance();
    
    // Configure BANANA effect (Chipmunks: +10 semitones)
    soundtouch_setSampleRate(bananaProcessor, 44100);
    soundtouch_setChannels(bananaProcessor, 1);
    soundtouch_setPitchSemiTones(bananaProcessor, 10);  // +10 semitones
    soundtouch_setRate(bananaProcessor, 1.0f);
    soundtouch_setTempo(bananaProcessor, 1.0f);
    soundtouch_setSetting(bananaProcessor, 0, 1);  // SETTING_USE_AA_FILTER
    
    // Configure DEVIL effect (Deep: -8 semitones)
    soundtouch_setSampleRate(devilProcessor, 44100);
    soundtouch_setChannels(devilProcessor, 1);
    soundtouch_setPitchSemiTones(devilProcessor, -8);  // -8 semitones
    soundtouch_setRate(devilProcessor, 1.0f);
    soundtouch_setTempo(devilProcessor, 1.0f);
    soundtouch_setSetting(devilProcessor, 0, 1);
    
    // Configure FEMALE effect (+4 semitones)
    soundtouch_setSampleRate(femaleProcessor, 44100);
    soundtouch_setChannels(femaleProcessor, 1);
    soundtouch_setPitchSemiTones(femaleProcessor, 4);   // +4 semitones
    soundtouch_setRate(femaleProcessor, 1.0f);
    soundtouch_setTempo(femaleProcessor, 1.0f);
    soundtouch_setSetting(femaleProcessor, 0, 1);
    
    // Configure MILITARY effect (No pitch change, just processing)
    soundtouch_setSampleRate(militaryProcessor, 44100);
    soundtouch_setChannels(militaryProcessor, 1);
    soundtouch_setPitchSemiTones(militaryProcessor, 0);  // No pitch change
    soundtouch_setRate(militaryProcessor, 1.0f);
    soundtouch_setTempo(militaryProcessor, 1.0f);
    soundtouch_setSetting(militaryProcessor, 0, 1);
    
    // Configure EKO effect (No pitch change, just delay)
    soundtouch_setSampleRate(ekoProcessor, 44100);
    soundtouch_setChannels(ekoProcessor, 1);
    soundtouch_setPitchSemiTones(ekoProcessor, 0);  // No pitch change
    soundtouch_setRate(ekoProcessor, 1.0f);
    soundtouch_setTempo(ekoProcessor, 1.0f);
    soundtouch_setSetting(ekoProcessor, 0, 1);
}

void VoiceEffects::initializeFilters()
{
    memset(&lowPass, 0, sizeof(lowPass));
    memset(&highPass, 0, sizeof(highPass));
    memset(&bandPass, 0, sizeof(bandPass));
}

void VoiceEffects::initializeChorus()
{
    chorus.delayBufferSize = 2048;
    chorus.delayBuffer.resize(chorus.delayBufferSize, 0.0f);
    chorus.writeIndex = 0;
    chorus.lfoPhase = 0.0f;
    chorus.lfoFreq = 0.5f;
    chorus.delayDepth = 0.002f; // 2ms modulation depth
}

void VoiceEffects::initializeEcho()
{
    echo.delayBufferSize = 44100; // 1 second at 44.1kHz
    echo.delayBuffer.resize(echo.delayBufferSize, 0.0f);
    echo.writeIndex = 0;
    echo.feedback = 0.4f;
    echo.wetLevel = 0.3f;
}

void VoiceEffects::processBanana(float* input, float* output, int bufferSize, float sampleRate)
{
    // BANANA EFFECT: Alvin & Chipmunks style using SoundTouch
    
    // Update sample rate if needed
    if (soundtouch_getSetting(bananaProcessor, 1) != static_cast<int>(sampleRate)) {
        soundtouch_setSampleRate(bananaProcessor, static_cast<uint>(sampleRate));
    }
    
    // Step 1: High-pass filter to remove low frequencies
    std::vector<float> filtered(bufferSize);
    applyHighPass(input, filtered.data(), bufferSize, 400.0f, sampleRate);
    
    // Step 2: Pitch shift using SoundTouch
    soundtouch_putSamples(bananaProcessor, filtered.data(), bufferSize);
    std::vector<float> pitchedBuffer(bufferSize);
    uint receivedSamples = soundtouch_receiveSamples(bananaProcessor, pitchedBuffer.data(), bufferSize);
    
    // If we didn't get enough samples, flush the processor
    if (receivedSamples < static_cast<uint>(bufferSize)) {
        soundtouch_flush(bananaProcessor);
        uint additionalSamples = soundtouch_receiveSamples(
            bananaProcessor, pitchedBuffer.data() + receivedSamples, bufferSize - receivedSamples);
        receivedSamples += additionalSamples;
    }
    
    // Step 3: Apply chorus for chipmunk effect
    std::vector<float> chorusBuffer(receivedSamples);
    applyChorus(pitchedBuffer.data(), chorusBuffer.data(), receivedSamples, sampleRate);
    
    // Step 4: Normalize and protect from clipping
    normalizeGain(chorusBuffer.data(), receivedSamples);
    protectClipping(chorusBuffer.data(), receivedSamples);
    
    // Copy to output (pad with zeros if needed)
    memset(output, 0, bufferSize * sizeof(float));
    memcpy(output, chorusBuffer.data(), receivedSamples * sizeof(float));
}

void VoiceEffects::processDevil(float* input, float* output, int bufferSize, float sampleRate)
{
    // DEVIL EFFECT: Deep, demonic voice using SoundTouch
    
    // Update sample rate if needed
    if (soundtouch_getSetting(devilProcessor, 1) != static_cast<int>(sampleRate)) {
        soundtouch_setSampleRate(devilProcessor, static_cast<uint>(sampleRate));
    }
    
    // Step 1: Pitch shift using SoundTouch
    soundtouch_putSamples(devilProcessor, input, bufferSize);
    std::vector<float> pitchedBuffer(bufferSize);
    uint receivedSamples = soundtouch_receiveSamples(devilProcessor, pitchedBuffer.data(), bufferSize);
    
    // If we didn't get enough samples, flush the processor
    if (receivedSamples < static_cast<uint>(bufferSize)) {
        soundtouch_flush(devilProcessor);
        uint additionalSamples = soundtouch_receiveSamples(
            devilProcessor, pitchedBuffer.data() + receivedSamples, bufferSize - receivedSamples);
        receivedSamples += additionalSamples;
    }
    
    // Step 2: Apply soft distortion (tanh clipping)
    std::vector<float> distorted(receivedSamples);
    distortion.drive = 1.5f;
    distortion.mix = 0.2f;
    applyDistortion(pitchedBuffer.data(), distorted.data(), receivedSamples);
    
    // Step 3: Low-pass filter for dark, muffled sound
    std::vector<float> filtered(receivedSamples);
    applyLowPass(distorted.data(), filtered.data(), receivedSamples, 4000.0f, sampleRate);
    
    // Step 4: Normalize and protect from clipping
    normalizeGain(filtered.data(), receivedSamples);
    protectClipping(filtered.data(), receivedSamples);
    
    // Copy to output (pad with zeros if needed)
    memset(output, 0, bufferSize * sizeof(float));
    memcpy(output, filtered.data(), receivedSamples * sizeof(float));
}

void VoiceEffects::processFemale(float* input, float* output, int bufferSize, float sampleRate)
{
    // FEMALE EFFECT: Natural female voice transformation using SoundTouch
    
    // Update sample rate if needed
    if (soundtouch_getSetting(femaleProcessor, 1) != static_cast<int>(sampleRate)) {
        soundtouch_setSampleRate(femaleProcessor, static_cast<uint>(sampleRate));
    }
    
    // Step 1: Pitch shift using SoundTouch
    soundtouch_putSamples(femaleProcessor, input, bufferSize);
    std::vector<float> pitchedBuffer(bufferSize);
    uint receivedSamples = soundtouch_receiveSamples(femaleProcessor, pitchedBuffer.data(), bufferSize);
    
    // If we didn't get enough samples, flush the processor
    if (receivedSamples < static_cast<uint>(bufferSize)) {
        soundtouch_flush(femaleProcessor);
        uint additionalSamples = soundtouch_receiveSamples(
            femaleProcessor, pitchedBuffer.data() + receivedSamples, bufferSize - receivedSamples);
        receivedSamples += additionalSamples;
    }
    
    // Step 2: Slight high-frequency boost for formant shift
    std::vector<float> filtered(receivedSamples);
    applyHighPass(pitchedBuffer.data(), filtered.data(), receivedSamples, 200.0f, sampleRate);
    
    // Step 3: Normalize gain
    normalizeGain(filtered.data(), receivedSamples);
    protectClipping(filtered.data(), receivedSamples);
    
    // Copy to output (pad with zeros if needed)
    memset(output, 0, bufferSize * sizeof(float));
    memcpy(output, filtered.data(), receivedSamples * sizeof(float));
}

void VoiceEffects::processMilitary(float* input, float* output, int bufferSize, float sampleRate)
{
    // MILITARY EFFECT: Walkie-talkie / radio communication style
    
    // Step 1: Band-pass filter (300Hz - 3kHz)
    std::vector<float> filtered(bufferSize);
    applyBandPass(input, filtered.data(), bufferSize, 300.0f, 3000.0f, sampleRate);
    
    // Step 2: Apply hard compressor/limiter
    compressor.threshold = 0.5f;
    compressor.ratio = 10.0f; // Hard limiting
    compressor.attackTime = 0.001f;
    compressor.releaseTime = 0.05f;
    std::vector<float> compressed(bufferSize);
    applyCompressor(filtered.data(), compressed.data(), bufferSize, sampleRate);
    
    // Step 3: Light saturation for radio feel
    std::vector<float> saturated(bufferSize);
    distortion.drive = 1.2f;
    distortion.mix = 0.15f;
    applyDistortion(compressed.data(), saturated.data(), bufferSize);
    
    // Step 4: Normalize and protect from clipping
    normalizeGain(saturated.data(), bufferSize);
    protectClipping(saturated.data(), bufferSize);
    
    // Copy to output
    memcpy(output, saturated.data(), bufferSize * sizeof(float));
}

void VoiceEffects::processEko(float* input, float* output, int bufferSize, float sampleRate)
{
    // EKO EFFECT: Echo/delay effect
    
    // Step 1: Apply echo effect
    std::vector<float> echoed(bufferSize);
    applyEcho(input, echoed.data(), bufferSize, sampleRate);
    
    // Step 2: Normalize and protect from clipping
    normalizeGain(echoed.data(), bufferSize);
    protectClipping(echoed.data(), bufferSize);
    
    // Copy to output
    memcpy(output, echoed.data(), bufferSize * sizeof(float));
}

void VoiceEffects::resetEffects()
{
    // Reset all SoundTouch processors
    soundtouch_clear(bananaProcessor);
    soundtouch_clear(devilProcessor);
    soundtouch_clear(femaleProcessor);
    soundtouch_clear(militaryProcessor);
    soundtouch_clear(ekoProcessor);
    
    // Reset all filter states
    memset(&lowPass, 0, sizeof(lowPass));
    memset(&highPass, 0, sizeof(highPass));
    memset(&bandPass, 0, sizeof(bandPass));
    
    // Clear chorus buffer
    std::fill(chorus.delayBuffer.begin(), chorus.delayBuffer.end(), 0.0f);
    chorus.writeIndex = 0;
    chorus.lfoPhase = 0.0f;
    
    // Reset compressor envelope
    compressor.envelope = 0.0f;
    compressor.gainReduction = 0.0f;
    
    // Clear echo buffer
    std::fill(echo.delayBuffer.begin(), echo.delayBuffer.end(), 0.0f);
    echo.writeIndex = 0;
}

// DSP processing functions implementation

void VoiceEffects::applyLowPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate)
{
    designLowPass(cutoff, sampleRate, lowPass);
    
    for (int i = 0; i < bufferSize; ++i) {
        float x = input[i];
        float y = (lowPass.b0 * x + lowPass.b1 * lowPass.x1 + lowPass.b2 * lowPass.x2 
                - lowPass.a1 * lowPass.y1 - lowPass.a2 * lowPass.y2) / lowPass.a0;
        
        lowPass.x2 = lowPass.x1;
        lowPass.x1 = x;
        lowPass.y2 = lowPass.y1;
        lowPass.y1 = y;
        
        output[i] = y;
    }
}

void VoiceEffects::applyHighPass(float* input, float* output, int bufferSize, float cutoff, float sampleRate)
{
    designHighPass(cutoff, sampleRate, highPass);
    
    for (int i = 0; i < bufferSize; ++i) {
        float x = input[i];
        float y = (highPass.b0 * x + highPass.b1 * highPass.x1 + highPass.b2 * highPass.x2 
                - highPass.a1 * highPass.y1 - highPass.a2 * highPass.y2) / highPass.a0;
        
        highPass.x2 = highPass.x1;
        highPass.x1 = x;
        highPass.y2 = highPass.y1;
        highPass.y1 = y;
        
        output[i] = y;
    }
}

void VoiceEffects::applyBandPass(float* input, float* output, int bufferSize, float lowFreq, float highFreq, float sampleRate)
{
    designBandPass(lowFreq, highFreq, sampleRate, bandPass);
    
    for (int i = 0; i < bufferSize; ++i) {
        float x = input[i];
        float y = (bandPass.b0 * x + bandPass.b1 * bandPass.x1 + bandPass.b2 * bandPass.x2 
                - bandPass.a1 * bandPass.y1 - bandPass.a2 * bandPass.y2) / bandPass.a0;
        
        bandPass.x2 = bandPass.x1;
        bandPass.x1 = x;
        bandPass.y2 = bandPass.y1;
        bandPass.y1 = y;
        
        output[i] = y;
    }
}

void VoiceEffects::applyChorus(float* input, float* output, int bufferSize, float sampleRate)
{
    float samplePeriod = 1.0f / sampleRate;
    
    for (int i = 0; i < bufferSize; ++i) {
        // LFO for delay modulation
        float lfo = sinf(chorus.lfoPhase) * 0.5f + 0.5f;
        float delaySamples = lfo * chorus.delayDepth * sampleRate;
        
        // Read from delay buffer with interpolation
        float readIndex = chorus.writeIndex - delaySamples;
        if (readIndex < 0) readIndex += chorus.delayBufferSize;
        
        int readIndexInt = static_cast<int>(readIndex);
        float frac = readIndex - readIndexInt;
        int nextIndex = (readIndexInt + 1) % chorus.delayBufferSize;
        
        float delayed = chorus.delayBuffer[readIndexInt] * (1.0f - frac) + 
                      chorus.delayBuffer[nextIndex] * frac;
        
        // Write to delay buffer
        chorus.delayBuffer[chorus.writeIndex] = input[i];
        chorus.writeIndex = (chorus.writeIndex + 1) % chorus.delayBufferSize;
        
        // Mix dry and wet signals
        output[i] = input[i] + delayed * 0.5f;
        
        // Update LFO phase
        chorus.lfoPhase += 2.0f * M_PI * chorus.lfoFreq * samplePeriod;
        if (chorus.lfoPhase >= 2.0f * M_PI) {
            chorus.lfoPhase -= 2.0f * M_PI;
        }
    }
}

void VoiceEffects::applyDistortion(float* input, float* output, int bufferSize)
{
    for (int i = 0; i < bufferSize; ++i) {
        float dry = input[i];
        float wet = tanhSoftClip(dry * distortion.drive);
        output[i] = dry * (1.0f - distortion.mix) + wet * distortion.mix;
    }
}

void VoiceEffects::applyCompressor(float* input, float* output, int bufferSize, float sampleRate)
{
    float attackCoeff = expf(-1.0f / (compressor.attackTime * sampleRate));
    float releaseCoeff = expf(-1.0f / (compressor.releaseTime * sampleRate));
    
    for (int i = 0; i < bufferSize; ++i) {
        float inputLevel = fabsf(input[i]);
        
        // Update envelope follower
        if (inputLevel > compressor.envelope) {
            compressor.envelope = inputLevel + (compressor.envelope - inputLevel) * attackCoeff;
        } else {
            compressor.envelope = inputLevel + (compressor.envelope - inputLevel) * releaseCoeff;
        }
        
        // Calculate gain reduction
        if (compressor.envelope > compressor.threshold) {
            float overThreshold = compressor.envelope - compressor.threshold;
            float targetGain = compressor.threshold + overThreshold / compressor.ratio;
            compressor.gainReduction = targetGain / compressor.envelope;
        } else {
            compressor.gainReduction = 1.0f;
        }
        
        output[i] = input[i] * compressor.gainReduction;
    }
}

void VoiceEffects::applyEcho(float* input, float* output, int bufferSize, float sampleRate)
{
    int delaySamples = static_cast<int>(0.3f * sampleRate); // 300ms delay
    
    for (int i = 0; i < bufferSize; ++i) {
        // Read from delay buffer
        int readIndex = echo.writeIndex - delaySamples;
        if (readIndex < 0) readIndex += echo.delayBufferSize;
        
        float delayed = echo.delayBuffer[readIndex];
        
        // Write to delay buffer (input + feedback)
        echo.delayBuffer[echo.writeIndex] = input[i] + delayed * echo.feedback;
        echo.writeIndex = (echo.writeIndex + 1) % echo.delayBufferSize;
        
        // Mix dry and wet signals
        output[i] = input[i] + delayed * echo.wetLevel;
    }
}

void VoiceEffects::normalizeGain(float* buffer, int bufferSize)
{
    // Find peak value
    float peak = 0.0f;
    for (int i = 0; i < bufferSize; ++i) {
        float abs = fabsf(buffer[i]);
        if (abs > peak) peak = abs;
    }
    
    // Normalize if peak is too high
    if (peak > 0.8f) {
        float gain = 0.8f / peak;
        for (int i = 0; i < bufferSize; ++i) {
            buffer[i] *= gain;
        }
    }
}

void VoiceEffects::protectClipping(float* buffer, int bufferSize)
{
    for (int i = 0; i < bufferSize; ++i) {
        if (buffer[i] > 0.95f) buffer[i] = 0.95f;
        else if (buffer[i] < -0.95f) buffer[i] = -0.95f;
    }
}

float VoiceEffects::tanhSoftClip(float x)
{
    // Soft clipping using tanh approximation
    if (x > 1.0f) return tanhf(x);
    else if (x < -1.0f) return tanhf(x);
    else return x * (1.5f - 0.5f * x * x);
}

void VoiceEffects::designLowPass(float cutoff, float sampleRate, FilterState& filter)
{
    float omega = 2.0f * M_PI * cutoff / sampleRate;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / sqrtf(2.0f);
    
    filter.b0 = (1.0f - cos_omega) / 2.0f;
    filter.b1 = 1.0f - cos_omega;
    filter.b2 = (1.0f - cos_omega) / 2.0f;
    filter.a0 = 1.0f + alpha;
    filter.a1 = -2.0f * cos_omega;
    filter.a2 = 1.0f - alpha;
}

void VoiceEffects::designHighPass(float cutoff, float sampleRate, FilterState& filter)
{
    float omega = 2.0f * M_PI * cutoff / sampleRate;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float alpha = sin_omega / sqrtf(2.0f);
    
    filter.b0 = (1.0f + cos_omega) / 2.0f;
    filter.b1 = -(1.0f + cos_omega);
    filter.b2 = (1.0f + cos_omega) / 2.0f;
    filter.a0 = 1.0f + alpha;
    filter.a1 = -2.0f * cos_omega;
    filter.a2 = 1.0f - alpha;
}

void VoiceEffects::designBandPass(float lowFreq, float highFreq, float sampleRate, FilterState& filter)
{
    // Simplified band-pass coefficients
    float bw = (highFreq - lowFreq) / sampleRate;
    float centerFreq = (lowFreq + highFreq) / 2.0f;
    float omega = 2.0f * M_PI * centerFreq / sampleRate;
    
    filter.b0 = bw;
    filter.b1 = 0.0f;
    filter.b2 = -bw;
    filter.a0 = 1.0f + bw;
    filter.a1 = -2.0f * cosf(omega);
    filter.a2 = 1.0f - bw;
}