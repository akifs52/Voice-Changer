#include "psola.h"
#include <cmath>
#include <algorithm>
#include <Qt>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PSOLA::PSOLA() : m_lastSample(0.0f), m_hpPrev(0.0f), m_lastInput(0.0f),
                   m_echoIndex1(0), m_echoIndex2(0), m_echoIndex3(0), m_lowPassState(0.0f)
{
}

PSOLA::~PSOLA()
{
}

float PSOLA::getDefaultPitchFactor(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 1.35f;
        case ROBOT:    return 0.95f;  // Sadece hafif pitch shift
        case DEVIL:    return 0.82f;
        case FEMALE:   return 1.3f;
        case COMBINE:  return 1.2f;
        case EKO:      return 1.0f;   // Echo için pitch değişimi yok
        default:       return 1.0f;
    }
}

float PSOLA::getDefaultFilterStrength(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 0.25f;
        case ROBOT:    return 0.6f;   // Daha güçlü filtering
        case DEVIL:    return 0.15f;
        case FEMALE:   return 0.3f;
        case COMBINE:  return 0.2f;
        case EKO:      return 0.05f;  // Minimal filtering for echo
        default:       return 0.0f;
    }
}

void PSOLA::process(int16_t* pcm, int sampleCount, int sampleRate, 
                   float pitchFactor, EffectType effect,
                   float filterStrength)
{
    if (!pcm || sampleCount <= 0 || sampleRate <= 0) return;
    if (pitchFactor <= 0.1f || pitchFactor > 5.0f) return;
    if (sampleCount < 200) return;

    // Use defaults if not specified
    if (filterStrength < 0.0f) {
        filterStrength = getDefaultFilterStrength(effect);
    }

    // -----------------------------
    // NEW: Noise gate and signal detection
    // -----------------------------
    
    // Convert to float without noise gate for cleaner sound
    std::vector<float> inputFloat(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
        float sample = pcm[i] / 32768.0f;

        // Simple hard limiting only - no noise gate
        if (sample > 0.9f) sample = 0.9f + (sample - 0.9f) * 0.3f;
        if (sample < -0.9f) sample = -0.9f + (sample + 0.9f) * 0.3f;

        inputFloat[i] = sample;
    }

    // NO input high-pass - causes dual voice effect
    
    // Minimal buffering for real-time processing
    std::vector<float> buffer(sampleCount, 0.0f);
    for (int i = 0; i < sampleCount; i++) {
        buffer[i] = inputFloat[i];
    }
    
    // NO edge smoothing - causes delay

    // Apply effect-specific processing
    if (effect == ROBOT) {
        applyRobotEffect(buffer, sampleCount, sampleRate, pitchFactor);
    } else {
        // Normal pitch shifting for other effects
        applyPitchShift(buffer, sampleCount, pitchFactor);
    }

    // Apply filtering if needed
    if (filterStrength > 0.0f) {
        applyEffectFilter(buffer, effect, filterStrength, sampleRate);
    }
    
    // ---- MAĞARA EKO ----
    if (effect == EKO) {

        applyCaveEcho(buffer, sampleCount, sampleRate);
    }
    
    // Enhanced output limiting with noise gate - SOFTER SETTINGS
    applyOutputLimiting(buffer, sampleCount);

    // Convert back to int16_t with careful scaling
    float maxVal = 0.0001f; // Avoid division by zero
    for (int i = 0; i < sampleCount; i++) {
        float val = std::abs(buffer[i]);
        if (val > maxVal) maxVal = val;
    }
    
    // Normalize and convert - ONLY WHEN NEEDED
    float gain = (maxVal > 0.6f) ? (0.95f / maxVal) : 1.0f;
    for (int i = 0; i < sampleCount; i++) {
        float sample = buffer[i] * gain;
        
        // Gentle qbound clamping - yüksek tizleri sınırla
        if (sample > 0.85f) {
            sample = 0.85f + (sample - 0.85f) * 0.3f;  // Yumuşak clamp
        } else if (sample < -0.85f) {
            sample = -0.85f + (sample + 0.85f) * 0.3f;  // Yumuşak clamp
        }
        
        pcm[i] = static_cast<int16_t>(sample * 32767.0f);
    }
    
    // Update last sample for continuity
    m_lastSample = buffer[sampleCount - 1];
}

void PSOLA::applyPitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor)
{
    if (pitchFactor == 1.0f) return;
    
    // Minimal latency pitch shifting - no padding
    std::vector<float> original = buffer;
    
    for (int i = 0; i < sampleCount; i++) {
        float srcPos = i * pitchFactor;
        int idx = static_cast<int>(srcPos);
        float frac = srcPos - idx;
        
        if (idx >= 1 && idx + 1 < (int)original.size()) {
            float y1 = original[idx];
            float y2 = original[idx + 1];
            
            // Linear interpolation for simplicity and speed
            float interpolated = y1 * (1.0f - frac) + y2 * frac;
            buffer[i] = interpolated;
        }
    }
}

void PSOLA::applyAutotune(std::vector<float>& buffer, int sampleCount, int sampleRate)
{
    // CRASH FIX: Add safety checks at start
    if (buffer.empty() || sampleCount <= 0 || sampleRate <= 0) {
        return;
    }
    
    // RAPÇİ için pentatonik ve blues notaları (daha doğal sound)
    const float notes[] = {
        130.81f, 146.83f, 155.56f, 174.61f, 196.00f,  // C3 - G3
        220.00f, 233.08f, 261.63f, 293.66f, 311.13f,  // A3 - D4
        329.63f, 349.23f, 369.99f, 392.00f, 415.30f,  // E4 - G#4
        440.00f, 466.16f, 493.88f, 523.25f            // A4 - C5
    };
    const int numNotes = sizeof(notes) / sizeof(notes[0]);
    
    const int frameSize = 256;  // Analysis frame size
    const int hopSize = 128;    // Hop size
    
    // CRASH FIX: No padding - use actual buffer size
    int bufferEnd = sampleCount;
    
    // CRASH FIX: Change loop condition for no padding
    for (int pos = 0; pos + frameSize < bufferEnd; pos += hopSize) {
        int end = std::min(pos + frameSize, bufferEnd);
        if (end - pos < 64) break;  // Minimum frame size
        
        // CRASH FIX: Validate frame extraction range
        if (end > static_cast<int>(buffer.size())) {
            break;
        }
        
        // Extract frame
        std::vector<float> frame(buffer.begin() + pos, buffer.begin() + end);
        
        // Estimate pitch
        float pitch = estimatePitchFrequency(frame, sampleRate);
        
        if (pitch > 80.0f && pitch < 1000.0f) {
            // Find closest note - RAPÇİ için daha hızlı ve agresif
            float closestNote = notes[0];
            float minDist = std::abs(pitch - notes[0]);
            
            for (int i = 1; i < numNotes; i++) {
                float dist = std::abs(pitch - notes[i]);
                if (dist < minDist) {
                    minDist = dist;
                    closestNote = notes[i];
                }
            }
            
            // Calculate correction factor
            float correction = closestNote / pitch;
            
            // Apply pitch correction with stronger windowing for rapper effect
            int frameLen = end - pos;
            for (int i = 0; i < frameLen && pos + i < sampleCount; i++) {
                // CRITICAL FIX: Bounds checking for buffer access
                int writeIdx = pos + i;
                if (writeIdx < 0 || writeIdx >= sampleCount)
                    continue;
                    
                float window = hanningWindow(i, frameLen);
                
                // Time-stretching based correction (basit versiyon)
                float origIdx = i / correction;
                int idx0 = static_cast<int>(origIdx);
                float frac = origIdx - idx0;
                
                // CRITICAL FIX: Bounds checking for frame access
                if (idx0 < 0 || idx0 + 1 >= frameLen)
                    continue;
                    
                float corrected = frame[idx0] * (1.0f - frac) + frame[idx0 + 1] * frac;
                // RAPÇİ için daha güçlü correction (0.9f yerine 0.8f)
                buffer[writeIdx] = buffer[writeIdx] * (1.0f - window * 0.8f) + 
                                 corrected * window * 0.8f;
            }
        }
    }
}

float PSOLA::estimatePitchFrequency(const std::vector<float>& frame, int sampleRate)
{
    if (frame.size() < 64) return 0.0f;
    
    // Auto-correlation based pitch detection
    int maxLag = sampleRate / 80;  // 80Hz minimum
    int minLag = sampleRate / 400; // 400Hz maximum
    
    std::vector<float> acf(maxLag + 1, 0.0f);
    
    // Calculate auto-correlation
    for (int lag = minLag; lag <= maxLag; lag++) {
        float sum = 0.0f;
        for (size_t i = 0; i < frame.size() - lag; i++) {
            sum += frame[i] * frame[i + lag];
        }
        acf[lag] = sum / (frame.size() - lag);
    }
    
    // Find first major peak after first zero-crossing
    bool foundZero = false;
    float maxVal = 0.0f;
    int bestLag = 0;
    
    for (int lag = minLag; lag <= maxLag; lag++) {
        if (!foundZero && acf[lag] < 0) {
            foundZero = true;
            continue;
        }
        
        if (foundZero && acf[lag] > maxVal) {
            maxVal = acf[lag];
            bestLag = lag;
        }
    }
    
    if (bestLag > 0 && maxVal > 0.01f) {
        return sampleRate / (float)bestLag;
    }
    
    return 0.0f;
}

void PSOLA::applyRobotEffect(std::vector<float>& buffer, int sampleCount, int sampleRate, float pitchFactor)
{
    // 1. Önce pitch shift uygula
    if (pitchFactor != 1.0f) {
        applyPitchShift(buffer, sampleCount, pitchFactor);
    }
    
    // 2. RAPÇİ EFKTİ: Sadece autotune uygula
    applyAutotune(buffer, sampleCount, sampleRate);
    
    // 3. Robotik formant ve ring modulation kaldırıldı - sadece autotune kalacak
}



void PSOLA::applyRobotFormant(std::vector<float>& buffer, int sampleCount, int sampleRate)
{
    // STATE RESET: Add safety check at beginning
    if (sampleRate <= 0 || buffer.size() < (size_t)sampleCount + 16)
        return;
        
    // Strong formant filtering for robotic effect
    float freq1 = 800.0f;   // First formant
    float freq2 = 1600.0f;  // Second formant
    float q = 0.7f;         // Quality factor
    
    // Two-pole bandpass filter implementation
    float w1 = 2.0f * M_PI * freq1 / sampleRate;
    float w2 = 2.0f * M_PI * freq2 / sampleRate;
    float alpha1 = sin(w1) / (2.0f * q);
    float alpha2 = sin(w2) / (2.0f * q);
    
    float a0_1 = 1.0f + alpha1;
    float a0_2 = 1.0f + alpha2;
    
    // Filter coefficients
    float b0_1 = alpha1 / a0_1;
    float b1_1 = 0.0f;
    float b2_1 = -alpha1 / a0_1;
    float a1_1 = -2.0f * cos(w1) / a0_1;
    float a2_1 = (1.0f - alpha1) / a0_1;
    
    float b0_2 = alpha2 / a0_2;
    float b1_2 = 0.0f;
    float b2_2 = -alpha2 / a0_2;
    float a1_2 = -2.0f * cos(w2) / a0_2;
    float a2_2 = (1.0f - alpha2) / a0_2;
    
    // Filter states
    float x1_1 = 0.0f, x2_1 = 0.0f, y1_1 = 0.0f, y2_1 = 0.0f;
    float x1_2 = 0.0f, x2_2 = 0.0f, y1_2 = 0.0f, y2_2 = 0.0f;
    
    for (int i = 16; i < sampleCount + 16; i++) {
        float x = buffer[i];
        
        // First formant filter
        float y = b0_1 * x + b1_1 * x1_1 + b2_1 * x2_1 - a1_1 * y1_1 - a2_1 * y2_1;
        x2_1 = x1_1; x1_1 = x; y2_1 = y1_1; y1_1 = y;
        
        // Second formant filter
        float z = b0_2 * y + b1_2 * x1_2 + b2_2 * x2_2 - a1_2 * y1_2 - a2_2 * y2_2;
        x2_2 = x1_2; x1_2 = y; y2_2 = y1_2; y1_2 = z;
        
        // Mix: 60% filtered, 40% dry for metallic robotic sound
        buffer[i] = z * 0.6f + x * 0.4f;
    }
}

void PSOLA::applyRingModulation(std::vector<float>& buffer, int sampleCount, int sampleRate, float modFreq)
{
    float phaseInc = 2.0f * M_PI * modFreq / sampleRate;
    
    for (int i = 16; i < sampleCount + 16; i++) {
        float mod = 0.5f + 0.5f * sin(m_ringPhase);
        // FIX: Prevent complete silence by ensuring minimum modulation
        if (mod < 0.1f) mod = 0.1f;
        buffer[i] *= mod;
        m_ringPhase += phaseInc;
        if (m_ringPhase > 2.0f * M_PI) m_ringPhase -= 2.0f * M_PI;
    }
}


void PSOLA::applyCaveEcho(std::vector<float>& buffer, int sampleCount, int sampleRate)
{
    float delayMs1 = 300.0f;
    float delayMs2 = 500.0f;
    float delayMs3 = 750.0f;
    float feedback1 = 0.7f;
    float feedback2 = 0.6f;
    float feedback3 = 0.5f;
    float mix = 0.8f;

    int delaySamples1 = static_cast<int>(delayMs1 * sampleRate / 1000.0f);
    int delaySamples2 = static_cast<int>(delayMs2 * sampleRate / 1000.0f);
    int delaySamples3 = static_cast<int>(delayMs3 * sampleRate / 1000.0f);

    if (delaySamples1 <= 0 || delaySamples2 <= 0 || delaySamples3 <= 0)
        return;

    if ((int)m_echoBuffer1.size() != delaySamples1)
        m_echoBuffer1.assign(delaySamples1, 0.0f);
    if ((int)m_echoBuffer2.size() != delaySamples2)
        m_echoBuffer2.assign(delaySamples2, 0.0f);
    if ((int)m_echoBuffer3.size() != delaySamples3)
        m_echoBuffer3.assign(delaySamples3, 0.0f);

    for (int i = 0; i < sampleCount; i++) {  // No padding - start from 0
        float dry = buffer[i];
        float echo1 = m_echoBuffer1[m_echoIndex1];
        float echo2 = m_echoBuffer2[m_echoIndex2];
        float echo3 = m_echoBuffer3[m_echoIndex3];

        float totalEcho = echo1 * 0.5f + echo2 * 0.3f + echo3 * 0.2f;
        buffer[i] = dry + totalEcho * mix;

        m_echoBuffer1[m_echoIndex1] = dry + echo1 * feedback1;
        m_echoBuffer2[m_echoIndex2] = dry + echo2 * feedback2;
        m_echoBuffer3[m_echoIndex3] = dry + echo3 * feedback3;

        m_echoIndex1 = (m_echoIndex1 + 1) % delaySamples1;
        m_echoIndex2 = (m_echoIndex2 + 1) % delaySamples2;
        m_echoIndex3 = (m_echoIndex3 + 1) % delaySamples3;
    }
}

void PSOLA::applyInputHighPass(std::vector<float>& buffer, int sampleRate, float cutoff)
{
    if (cutoff <= 0.0f) return;
    
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float alpha = rc / (rc + dt);
    
    float prev = m_hpPrev;
    
    // FIX: Reset filter state at buffer beginning
    if (!buffer.empty()) {
        m_lastInput = buffer[0];
    }
    
    for (size_t i = 0; i < buffer.size(); i++) {
        float x = buffer[i];
        float y = alpha * (prev + x - m_lastInput);
        buffer[i] = y;
        prev = y;
        m_lastInput = x;
    }
    
    m_hpPrev = prev;
}

void PSOLA::applyEffectFilter(std::vector<float>& buffer, EffectType effect, 
                             float strength, int sampleRate)
{
    switch (effect) {
        case BANANA:
            // BANANA için filtering kaldırıldı - temiz gibi EKO
            break;
        case ROBOT:
            // Robot already has its own filtering
            break;
        case DEVIL:
            // DEVIL için filtering kaldırıldı - temiz gibi EKO
            break;
        case FEMALE:
            // FEMALE için filtering kaldırıldı - robotik ses gitmesi için
            break;
        default:
            if (strength > 0.0f) {
                applyHighPassFilter(buffer, strength, 150.0f, sampleRate);
            }
            break;
    }
}

void PSOLA::applyHighPassFilter(std::vector<float>& buffer, float strength, 
                               float cutoff, int sampleRate)
{
    if (strength <= 0.0f || cutoff <= 0.0f) return;
    
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float alpha = rc / (rc + dt);
    
    float prev = 0.0f;
    float lastIn = buffer[0];  // No padding - start from 0
    
    for (size_t i = 0; i < buffer.size(); i++) {  // No padding - start from 0
        float x = buffer[i];
        float y = alpha * (prev + x - lastIn);
        
        // Mix with original based on strength
        buffer[i] = x * (1.0f - strength) + y * strength;
        
        prev = y;
        lastIn = x;
    }
}

void PSOLA::applyLowPassFilter(std::vector<float>& buffer, float strength,
                              float cutoff, int sampleRate)
{
    if (strength <= 0.0f || cutoff <= 0.0f) return;
    
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float alpha = dt / (rc + dt);
    
    float prev = buffer[0];  // No padding - start from 0
    
    for (size_t i = 0; i < buffer.size(); i++) {  // No padding - start from 0
        float x = buffer[i];
        float y = prev + alpha * (x - prev);
        
        // Mix with original based on strength
        buffer[i] = x * (1.0f - strength) + y * strength;
        
        prev = y;
    }
}

void PSOLA::applyOutputLimiting(std::vector<float>& buffer, int sampleCount)
{
    // Simple limiting for BANANA and DEVIL - no complex processing
    for (int i = 0; i < sampleCount; i++) {
        float sample = buffer[i];
        
        // Gentle hard limiting only
        if (sample > 0.9f) {
            sample = 0.9f + (sample - 0.9f) * 0.5f;
        } else if (sample < -0.9f) {
            sample = -0.9f + (sample + 0.9f) * 0.5f;
        }
        
        buffer[i] = sample;
    }
}

float PSOLA::hanningWindow(int n, int N)
{
    return 0.5f * (1.0f - cosf(2.0f * M_PI * n / (N - 1)));
}

