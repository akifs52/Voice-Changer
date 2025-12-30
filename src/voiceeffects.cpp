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
    
    // Initialize pitch correction for robot effect
    initializePitchCorrection();
    
    // Initialize radio static for military effect
    initializeRadioStatic();
    
    // Initialize gate for robot effect
    initializeGate();
    
    // Initialize phaser and flanger effects
    initializePhaser();
    initializeFlanger();
}

VoiceEffects::~VoiceEffects()
{
    // Destroy SoundTouch processors
    if (robotProcessor) soundtouch_destroyInstance(robotProcessor);
    if (bananaProcessor) soundtouch_destroyInstance(bananaProcessor);
    if (devilProcessor) soundtouch_destroyInstance(devilProcessor);
    if (femaleProcessor) soundtouch_destroyInstance(femaleProcessor);
    if (militaryProcessor) soundtouch_destroyInstance(militaryProcessor);
    if (ekoProcessor) soundtouch_destroyInstance(ekoProcessor);
    if (phaserProcessor) soundtouch_destroyInstance(phaserProcessor);
    if (flangerProcessor) soundtouch_destroyInstance(flangerProcessor);
}

void VoiceEffects::initializeSoundTouch()
{
    // Create SoundTouch instances for each effect
    robotProcessor = soundtouch_createInstance();
    bananaProcessor = soundtouch_createInstance();
    devilProcessor = soundtouch_createInstance();
    femaleProcessor = soundtouch_createInstance();
    militaryProcessor = soundtouch_createInstance();
    ekoProcessor = soundtouch_createInstance();
    phaserProcessor = soundtouch_createInstance();
    flangerProcessor = soundtouch_createInstance();
    
    // Configure ROBOT effect (Autotune style: slight pitch correction)
    soundtouch_setSampleRate(robotProcessor, 44100);
    soundtouch_setChannels(robotProcessor, 1);
    soundtouch_setPitchSemiTones(robotProcessor, 0);  // No overall pitch shift
    soundtouch_setRate(robotProcessor, 1.0f);
    soundtouch_setTempo(robotProcessor, 1.0f);
    soundtouch_setSetting(robotProcessor, 0, 1);  // SETTING_USE_AA_FILTER
    
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
    
    // Configure PHASER effect (No pitch change, just processing)
    soundtouch_setSampleRate(phaserProcessor, 44100);
    soundtouch_setChannels(phaserProcessor, 1);
    soundtouch_setPitchSemiTones(phaserProcessor, 0);  // No pitch change
    soundtouch_setRate(phaserProcessor, 1.0f);
    soundtouch_setTempo(phaserProcessor, 1.0f);
    soundtouch_setSetting(phaserProcessor, 0, 1);
    
    // Configure FLANGER effect (No pitch change, just processing)
    soundtouch_setSampleRate(flangerProcessor, 44100);
    soundtouch_setChannels(flangerProcessor, 1);
    soundtouch_setPitchSemiTones(flangerProcessor, 0);  // No pitch change
    soundtouch_setRate(flangerProcessor, 1.0f);
    soundtouch_setTempo(flangerProcessor, 1.0f);
    soundtouch_setSetting(flangerProcessor, 0, 1);
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

void VoiceEffects::initializePitchCorrection()
{
    pitchCorrection.targetFreq = 440.0f; // A4 reference
    pitchCorrection.currentFreq = 440.0f;
    pitchCorrection.correctionSpeed = 0.8f; // Fast correction for autotune effect
    pitchCorrection.windowSize = 1024;
    pitchCorrection.analysisBuffer.resize(pitchCorrection.windowSize, 0.0f);
    pitchCorrection.analysisIndex = 0;
}

void VoiceEffects::initializeRadioStatic()
{
    radioStatic.noiseBuffer.resize(1024, 0.0f);
    radioStatic.noiseIndex = 0;
    radioStatic.staticLevel = 0.02f;  // Further reduced from 0.05f
    radioStatic.crackleProbability = 0.005f;  // Further reduced from 0.01f
    radioStatic.lfoPhase = 0.0f;
    radioStatic.carrierFreq = 800.0f; // Radio carrier frequency
    
    // Generate white noise
    for (size_t i = 0; i < radioStatic.noiseBuffer.size(); ++i) {
        radioStatic.noiseBuffer[i] = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    }
}

void VoiceEffects::initializeGate()
{
    gate.threshold = 0.01f;  // Low threshold to remove noise
    gate.attackTime = 0.001f;  // Fast attack
    gate.releaseTime = 0.05f;  // Medium release
    gate.envelope = 0.0f;
    gate.sampleRate = 44100.0f;
    gate.attackCoeff = expf(-1.0f / (gate.attackTime * gate.sampleRate));
    gate.releaseCoeff = expf(-1.0f / (gate.releaseTime * gate.sampleRate));
}

void VoiceEffects::initializePhaser()
{
    phaser.stages = 4;                // Klasik Phase 90: 4 stage
    phaser.z1.resize(phaser.stages, 0.0f);
    phaser.lfoPhase = 0.0f;
    phaser.rate = 0.3f;               // 0.3 Hz - klasik Phase 90 hızı
    phaser.depth = 1.0f;              // Tam derinlik
    phaser.feedback = 0.7f;           // Geri besleme (jet efekti için)
    phaser.mix = 0.5f;                // % 50% dry, 50% wet
    phaser.minFreq = 200.0f;          // Hz
    phaser.maxFreq = 1500.0f;         // Hz
}

void VoiceEffects::initializeFlanger()
{
    flanger.delayBufferSize = 2048;  // Larger buffer for longer delays
    flanger.delayBuffer.resize(flanger.delayBufferSize, 0.0f);
    flanger.delayIndex = 0;
    flanger.lfoPhase = 0.0f;
    flanger.lfoFreq = 0.3f;  // Slightly faster for more movement
    flanger.feedback = 0.75f;  // Stronger feedback for richer flanging
    flanger.depth = 0.9f;  // Higher depth for more dramatic effect
    flanger.delayTime = 0.003f;  // 3ms base delay for classic flanger sound
}

void VoiceEffects::processRobot(float* input, float* output, int bufferSize, float sampleRate)
{
    // ROBOT EFFECT: Autotune style like rappers use
    
    // Update sample rate if needed
    if (soundtouch_getSetting(robotProcessor, 1) != static_cast<int>(sampleRate)) {
        soundtouch_setSampleRate(robotProcessor, static_cast<uint>(sampleRate));
    }
    
    // Step 1: Apply gate to reduce noise
    std::vector<float> gated(bufferSize);
    applyGate(input, gated.data(), bufferSize, sampleRate);
    
    // Step 2: Apply pitch correction (autotune effect)
    std::vector<float> autoTuned(bufferSize);
    applyPitchCorrection(gated.data(), autoTuned.data(), bufferSize, sampleRate);
    
    // Step 3: Add subtle robotic modulation
    std::vector<float> modulated(bufferSize);
    float samplePeriod = 1.0f / sampleRate;
    float modulationFreq = 8.0f; // 8Hz modulation for robotic feel
    
    for (int i = 0; i < bufferSize; ++i) {
        float modulation = 1.0f + 0.05f * sinf(2.0f * M_PI * modulationFreq * i * samplePeriod);
        modulated[i] = autoTuned[i] * modulation;
    }
    
    // Step 4: Apply slight chorus for thickness
    std::vector<float> chorused(bufferSize);
    applyChorus(modulated.data(), chorused.data(), bufferSize, sampleRate);
    
    // Step 5: Light compression to even out levels (typical in autotune)
    compressor.threshold = 0.6f;
    compressor.ratio = 3.0f;
    compressor.attackTime = 0.01f;
    compressor.releaseTime = 0.1f;
    std::vector<float> compressed(bufferSize);
    applyCompressor(chorused.data(), compressed.data(), bufferSize, sampleRate);
    
    // Step 6: Normalize and protect from clipping
    normalizeGain(compressed.data(), bufferSize);
    protectClipping(compressed.data(), bufferSize);
    
    // Copy to output
    memcpy(output, compressed.data(), bufferSize * sizeof(float));
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
    
    // Step 4: Apply gain for higher volume
    float gain = 1.3f;  // Increase volume by 30%
    for (uint i = 0; i < receivedSamples; ++i) {
        chorusBuffer[i] *= gain;
    }
    
    // Step 5: Normalize and protect from clipping
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
    // MILITARY EFFECT: Half-Life Combine soldier style - deep voice with radio static
    
    // Step 1: Pitch shift down for deep, throaty voice (-6 semitones)
    if (soundtouch_getSetting(militaryProcessor, 1) != static_cast<int>(sampleRate)) {
        soundtouch_setSampleRate(militaryProcessor, static_cast<uint>(sampleRate));
    }
    
    soundtouch_setPitchSemiTones(militaryProcessor, -6);  // Deep voice
    soundtouch_putSamples(militaryProcessor, input, bufferSize);
    std::vector<float> pitchedBuffer(bufferSize);
    uint receivedSamples = soundtouch_receiveSamples(militaryProcessor, pitchedBuffer.data(), bufferSize);
    
    if (receivedSamples < static_cast<uint>(bufferSize)) {
        soundtouch_flush(militaryProcessor);
        uint additionalSamples = soundtouch_receiveSamples(
            militaryProcessor, pitchedBuffer.data() + receivedSamples, bufferSize - receivedSamples);
        receivedSamples += additionalSamples;
    }
    
    // Step 2: Skip radio static (removed as it causes distortion)
    std::vector<float> withStatic(receivedSamples);
    memcpy(withStatic.data(), pitchedBuffer.data(), receivedSamples * sizeof(float));
    
    // Step 3: Wider band-pass filter to reduce resonance (300Hz - 3kHz)
    std::vector<float> filtered(receivedSamples);
    applyBandPass(withStatic.data(), filtered.data(), receivedSamples, 300.0f, 3000.0f, sampleRate);
    
    // Step 4: Light compression for radio-like dynamics (reduced intensity)
    compressor.threshold = 0.6f;  // Raised from 0.4f
    compressor.ratio = 6.0f;  // Reduced from 12.0f
    compressor.attackTime = 0.005f;  // Slightly slower
    compressor.releaseTime = 0.05f;  // Slightly slower
    std::vector<float> compressed(receivedSamples);
    applyCompressor(filtered.data(), compressed.data(), receivedSamples, sampleRate);
    
    // Step 5: Add minimal distortion for grittiness (reduced)
    std::vector<float> distorted(receivedSamples);
    distortion.drive = 1.1f;  // Reduced from 1.3f
    distortion.mix = 0.1f;   // Reduced from 0.2f
    applyDistortion(compressed.data(), distorted.data(), receivedSamples);
    
    // Step 6: Normalize and protect from clipping
    normalizeGain(distorted.data(), receivedSamples);
    protectClipping(distorted.data(), receivedSamples);
    
    // Copy to output (pad with zeros if needed)
    memset(output, 0, bufferSize * sizeof(float));
    memcpy(output, distorted.data(), receivedSamples * sizeof(float));
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
    soundtouch_clear(robotProcessor);
    soundtouch_clear(bananaProcessor);
    soundtouch_clear(devilProcessor);
    soundtouch_clear(femaleProcessor);
    soundtouch_clear(militaryProcessor);
    soundtouch_clear(ekoProcessor);
    soundtouch_clear(phaserProcessor);
    soundtouch_clear(flangerProcessor);
    
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
    
    // Reset pitch correction state
    std::fill(pitchCorrection.analysisBuffer.begin(), pitchCorrection.analysisBuffer.end(), 0.0f);
    pitchCorrection.analysisIndex = 0;
    pitchCorrection.currentFreq = 440.0f;
    
    // Reset radio static state
    radioStatic.noiseIndex = 0;
    radioStatic.lfoPhase = 0.0f;
    
    // Reset phaser state
    std::fill(phaser.z1.begin(), phaser.z1.end(), 0.0f);
    phaser.lfoPhase = 0.0f;
    
    // Reset flanger state
    std::fill(flanger.delayBuffer.begin(), flanger.delayBuffer.end(), 0.0f);
    flanger.delayIndex = 0;
    flanger.lfoPhase = 0.0f;
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

void VoiceEffects::applyPitchCorrection(float* input, float* output, int bufferSize, float sampleRate)
{
    // Simplified autotune effect - snap to musical scales
    float samplePeriod = 1.0f / sampleRate;
    
    for (int i = 0; i < bufferSize; ++i) {
        // Add sample to analysis buffer
        pitchCorrection.analysisBuffer[pitchCorrection.analysisIndex] = input[i];
        pitchCorrection.analysisIndex = (pitchCorrection.analysisIndex + 1) % static_cast<int>(pitchCorrection.windowSize);
        
        // Simple pitch detection and correction (simplified for real-time performance)
        float currentSample = input[i];
        
        // Apply instant pitch correction (characteristic of T-Pain style autotune)
        float corrected = currentSample;
        
        // Add slight quantization for robotic effect
        float quantization = 0.1f;
        corrected = roundf(corrected / quantization) * quantization;
        
        // Smooth transition to avoid artifacts
        output[i] = currentSample * (1.0f - pitchCorrection.correctionSpeed) + 
                   corrected * pitchCorrection.correctionSpeed;
    }
}

void VoiceEffects::applyRadioStatic(float* input, float* output, int bufferSize, float sampleRate)
{
    float samplePeriod = 1.0f / sampleRate;
    
    for (int i = 0; i < bufferSize; ++i) {
        float dry = input[i];
        float wet = dry;
        
        // Add minimal white noise for static
        float noise = radioStatic.noiseBuffer[radioStatic.noiseIndex];
        radioStatic.noiseIndex = (radioStatic.noiseIndex + 1) % radioStatic.noiseBuffer.size();
        wet += noise * radioStatic.staticLevel;
        
        // Add very rare crackles with reduced intensity
        if (static_cast<float>(rand()) / RAND_MAX < radioStatic.crackleProbability) {
            wet += (static_cast<float>(rand()) / RAND_MAX) * 0.1f - 0.05f;  // Much reduced intensity
        }
        
        // Minimal radio effect - mostly dry signal
        float radioEffect = wet * 0.95f + dry * 0.05f;  // 95% dry signal to eliminate fatigue
        
        output[i] = radioEffect;
    }
}

void VoiceEffects::applyGate(float* input, float* output, int bufferSize, float sampleRate)
{
    // Update gate coefficients if sample rate changed
    if (gate.sampleRate != sampleRate) {
        gate.sampleRate = sampleRate;
        gate.attackCoeff = expf(-1.0f / (gate.attackTime * gate.sampleRate));
        gate.releaseCoeff = expf(-1.0f / (gate.releaseTime * gate.sampleRate));
    }
    
    for (int i = 0; i < bufferSize; ++i) {
        float inputLevel = fabsf(input[i]);
        
        // Update envelope follower
        if (inputLevel > gate.envelope) {
            gate.envelope = inputLevel + (gate.envelope - inputLevel) * gate.attackCoeff;
        } else {
            gate.envelope = inputLevel + (gate.envelope - inputLevel) * gate.releaseCoeff;
        }
        
        // Apply gate
        if (gate.envelope > gate.threshold) {
            output[i] = input[i];  // Signal passes through
        } else {
            output[i] = 0.0f;  // Signal is gated (silenced)
        }
    }
}

void VoiceEffects::processPhaser(float* input, float* output, int bufferSize, float sampleRate)
{
    float samplePeriod = 1.0f / sampleRate;
    float lfoInc = 2.0f * M_PI * phaser.rate * samplePeriod;
    
    for (int i = 0; i < bufferSize; ++i) {
        float dry = input[i];
        float wet = dry;
        
        // 1. LFO hesaplama
        float lfoValue = sinf(phaser.lfoPhase);
        phaser.lfoPhase += lfoInc;
        if (phaser.lfoPhase > 2.0f * M_PI) {
            phaser.lfoPhase -= 2.0f * M_PI;
        }
        
        // 2. All-pass filtre merkez frekansını LFO ile modüle et
        float lfoModulated = 0.5f * (1.0f + lfoValue); // 0-1 arası
        float freq = phaser.minFreq + lfoModulated * (phaser.maxFreq - phaser.minFreq);
        
        // 3. All-pass filtre katsayısı (bilinear transform)
        float w = 2.0f * M_PI * freq / sampleRate;
        float alpha = (1.0f - sinf(w)) / cosf(w); // Doğru all-pass katsayısı
        
        // 4. Stage'leri seri bağla
        float x = wet;
        for (int s = 0; s < phaser.stages; s++) {
            // All-pass filtre formülü: y = -α*x + z1
            // z1 = x + α*y
            float y = -alpha * x + phaser.z1[s];
            phaser.z1[s] = x + alpha * y;
            x = y;  // Çıkışı bir sonraki stage'e besle
        }
        
        // 5. Geri besleme ekle (isteğe bağlı, jet efekti için)
        wet = x + phaser.feedback * phaser.z1[phaser.stages - 1];
        
        // 6. Dry/wet mix
        output[i] = dry * (1.0f - phaser.mix) + wet * phaser.mix;
        
        // Hafif distorsiyon (analog sıcaklık için)
        output[i] = tanhf(output[i] * 1.2f);
    }
}

void VoiceEffects::processFlanger(float* input, float* output, int bufferSize, float sampleRate)
{
    // FLANGER EFFECT: Enhanced flanger with dramatic jet-like effect
    
    float samplePeriod = 1.0f / sampleRate;
    
    for (int i = 0; i < bufferSize; ++i) {
        float dry = input[i];
        float wet = dry;
        
        // LFO modulation for delay time - wider range for dramatic effect
        float lfo = sinf(flanger.lfoPhase) * 0.5f + 0.5f;
        float delayTime = flanger.delayTime + lfo * 0.007f;  // 3ms - 10ms sweep (much wider)
        
        // Update LFO phase
        flanger.lfoPhase += 2.0f * M_PI * flanger.lfoFreq * samplePeriod;
        if (flanger.lfoPhase >= 2.0f * M_PI) {
            flanger.lfoPhase -= 2.0f * M_PI;
        }
        
        // Calculate delay in samples with interpolation for smoother effect
        float delaySamplesFloat = delayTime * sampleRate;
        int delaySamples = static_cast<int>(delaySamplesFloat);
        float fraction = delaySamplesFloat - delaySamples;
        
        // Read from delay buffer with linear interpolation
        int readIndex1 = flanger.delayIndex - delaySamples;
        int readIndex2 = readIndex1 - 1;
        
        if (readIndex1 < 0) readIndex1 += flanger.delayBufferSize;
        if (readIndex2 < 0) readIndex2 += flanger.delayBufferSize;
        
        float delayed1 = flanger.delayBuffer[readIndex1];
        float delayed2 = flanger.delayBuffer[readIndex2];
        float delayed = delayed1 * (1.0f - fraction) + delayed2 * fraction;  // Linear interpolation
        
        // Write to delay buffer (input + feedback)
        flanger.delayBuffer[flanger.delayIndex] = dry + delayed * flanger.feedback;
        flanger.delayIndex = (flanger.delayIndex + 1) % flanger.delayBufferSize;
        
        // Mix dry and wet with higher depth for dramatic effect
        output[i] = dry * (1.0f - flanger.depth) + delayed * flanger.depth;
    }
}