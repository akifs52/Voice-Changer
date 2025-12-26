#include "psola.h"
#include <cmath>
#include <algorithm>
#include <Qt>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PSOLA::PSOLA() : m_lastSample(0.0f),
                   m_echoIndex1(0), m_echoIndex2(0)
{
}

PSOLA::~PSOLA()
{
}

float PSOLA::getDefaultPitchFactor(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 1.50f;  // Çok hafif
        case ROBOT:    return 0.98f;  // Neredeyse aynı
        case DEVIL:    return 0.75;  // Hafif düşük
        case FEMALE:   return 1.20f;  // Hafif yüksek
        case COMBINE:  return 1.0f;
        case EKO:      return 1.0f;
        default:       return 1.0f;
    }
}

float PSOLA::getDefaultFilterStrength(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 0.1f;   // Çok hafif filtering
        case ROBOT:    return 0.6f;
        case DEVIL:    return 0.08f;  // Minimum filtering
        case FEMALE:   return 0.12f;  // Hafif filtering
        case COMBINE:  return 0.15f;
        case EKO:      return 0.0f;   // No filtering for echo
        default:       return 0.0f;
    }
}

void PSOLA::applyPureEchoOnly(std::vector<float>& buffer, int sampleCount, int sampleRate)
{
    // SADECE echo, pitch shift YOK
    applyCaveEcho(buffer, sampleCount, sampleRate);
    
    // Çok hafif output limiting
    for (int i = 0; i < sampleCount; i++) {
        float sample = buffer[i];
        if (sample > 0.95f) sample = 0.95f;
        if (sample < -0.95f) sample = -0.95f;
        buffer[i] = sample;
    }
}

void PSOLA::process(int16_t* pcm, int sampleCount, int sampleRate, 
                   float pitchFactor, EffectType effect,
                   float)
{
    if (!pcm || sampleCount <= 0 || sampleRate <= 0) return;
    
    // 1. Convert to float
    std::vector<float> buffer(sampleCount);
    for (int i = 0; i < sampleCount; i++) {
        buffer[i] = pcm[i] / 32768.0f;
    }
    
    // 2. PITCH SHIFT - use Hermite interpolation instead of linear
    if (pitchFactor != 1.0f && effect != EKO) {
        // Pitch faktörünü sınırla (dar aralık daha stabil)
        pitchFactor = std::clamp(pitchFactor, 0.85f, 1.15f);
        
        // Noise gate (gürültüyü sustur)
        applyNoiseGate(buffer);
        
        // Önce hafif low-pass (anti-alias)
        applyGentleLowPass(buffer, sampleCount, 0.0f);
        
        // Hermite interpolation (daha stabil)
        applySimplePitchShift(buffer, sampleCount, pitchFactor);
        
        // DC blocker (dip gürültüsünü keser)
        applyDCBlocker(buffer);
        
        // RMS normalize (patlamayı keser)
        normalizeRMS(buffer);
        
        // Edge fade (çıtlamayı keser)
        applyEdgeFade(buffer);
    }
    
    // 3. Efektlere göre ÇOK HAFİF işlemler
    switch (effect) {
        case EKO:
            applyPureEchoOnly(buffer, sampleCount, sampleRate);
            break;
            
        case ROBOT:
            // Robot için sadece autotune
            applyAutotune(buffer, sampleCount, sampleRate);
            break;
            
        case DEVIL:
            // Devil için hafif distortion
            for (int i = 0; i < sampleCount; i++) {
                float x = buffer[i];
                // Cubic soft clipping
                if (x > 0.5f) {
                    x = 0.5f + (x - 0.5f) * 0.7f;
                } else if (x < -0.5f) {
                    x = -0.5f + (x + 0.5f) * 0.7f;
                }
                buffer[i] = x;
            }
            break;
            
        case BANANA:
        case FEMALE:
        case COMBINE:
            // Sadece pitch shift yeterli, başka işlem yok
            break;
    }
    
    // 4. Geliştirmiş limiter (daha erken devreye girer)
    for (int i = 0; i < sampleCount; i++) {
        float sample = buffer[i];
        
        // Daha agresif soft clipping
        sample = tanhf(sample * 1.2f);
        
        pcm[i] = static_cast<int16_t>(sample * 32767.0f);
    }
}

void PSOLA::applyUltraCleanPitchShift(std::vector<float>& buffer, int sampleCount, 
                                     float pitchFactor)
{
    if (pitchFactor == 1.0f) return;
    
    // 1. Önce anti-aliasing filtresi UYGULA
    if (pitchFactor > 1.05f) {
        applyStrongAntiAliasFilter(buffer, sampleCount, pitchFactor);
    }
    
    // 2. Çok basit ama stabil linear interpolation
    std::vector<float> original = buffer;
    
    for (int i = 0; i < sampleCount; i++) {
        float srcPos = i * pitchFactor;
        int idx = static_cast<int>(srcPos);
        
        if (idx < 0) {
            buffer[i] = 0.0f;
            continue;
        }
        
        if (idx >= sampleCount - 1) {
            if (idx < sampleCount) {
                buffer[i] = original[idx];
            } else {
                buffer[i] = 0.0f;
            }
            continue;
        }
        
        float frac = srcPos - idx;
        buffer[i] = original[idx] * (1.0f - frac) + original[idx + 1] * frac;
    }
}

void PSOLA::applyStrongAntiAliasFilter(std::vector<float>& buffer, int sampleCount, 
                                      float)
{
    // Simple two-pass moving average filter (çok temiz)
    std::vector<float> temp(sampleCount);
    
    // İlk pass
    for (int i = 1; i < sampleCount - 1; i++) {
        temp[i] = (buffer[i-1] + buffer[i] + buffer[i+1]) / 3.0f;
    }
    temp[0] = buffer[0];
    temp[sampleCount-1] = buffer[sampleCount-1];
    
    // İkinci pass
    for (int i = 1; i < sampleCount - 1; i++) {
        buffer[i] = (temp[i-1] + temp[i] + temp[i+1]) / 3.0f;
    }
}

void PSOLA::applyGentleLowPass(std::vector<float>& buffer, int sampleCount, float)
{
    // One-pole low-pass filter (çok yumuşak)
    static float prev = 0.0f;
    float alpha = 0.1f; // Sabit, basit filtre
    
    for (int i = 0; i < sampleCount; i++) {
        float x = buffer[i];
        float y = prev + alpha * (x - prev);
        buffer[i] = y;
        prev = y;
    }
}

void PSOLA::applyCleanPitchShift(std::vector<float>& buffer, int sampleCount, 
                                 float pitchFactor, EffectType)
{
    if (pitchFactor == 1.0f || pitchFactor <= 0.0f) return;
    
    // OLA (Overlap-Add) tabanlı pitch shifting - daha temiz
    const int windowSize = 512;
    const int hopSize = 256;
    
    if (sampleCount < windowSize * 2) {
        // Küçük buffer için basit interpolation
        applySimplePitchShift(buffer, sampleCount, pitchFactor);
        return;
    }
    
    std::vector<float> output(sampleCount, 0.0f);
    std::vector<float> window(windowSize);
    
    // Hanning window oluştur
    for (int i = 0; i < windowSize; i++) {
        window[i] = 0.5f * (1.0f - cosf(2.0f * M_PI * i / (windowSize - 1)));
    }
    
    int inputHop = hopSize;
    // int outputHop = static_cast<int>(hopSize * pitchFactor); // Unused variable
    
    // Overlap-Add ile pitch shifting
    for (int pos = 0; pos + windowSize <= sampleCount; pos += inputHop) {
        int outputPos = static_cast<int>(pos * pitchFactor);
        
        if (outputPos + windowSize > sampleCount) break;
        
        // Window uygula
        for (int i = 0; i < windowSize; i++) {
            int srcIdx = pos + i;
            int dstIdx = outputPos + i;
            
            if (srcIdx < sampleCount && dstIdx < sampleCount) {
                float windowedSample = buffer[srcIdx] * window[i];
                output[dstIdx] += windowedSample;
            }
        }
    }
    
    // Normalizasyon uygula
    float maxVal = 0.001f;
    for (int i = 0; i < sampleCount; i++) {
        if (fabs(output[i]) > maxVal) maxVal = fabs(output[i]);
    }
    
    if (maxVal > 1.0f) {
        float gain = 0.95f / maxVal;
        for (int i = 0; i < sampleCount; i++) {
            buffer[i] = output[i] * gain;
        }
    } else {
        buffer = output;
    }
}

void PSOLA::applySimplePitchShift(std::vector<float>& buffer, int sampleCount, 
                                  float pitchFactor)
{
    std::vector<float> original = buffer;
    
    for (int i = 0; i < sampleCount; i++) {
        float srcPos = i * pitchFactor;
        int idx0 = static_cast<int>(srcPos);
        
        if (idx0 < 0 || idx0 >= sampleCount - 3) {
            buffer[i] = 0.0f;
            continue;
        }
        
        float frac = srcPos - idx0;
        
        // 4-point, 3rd-order Hermite interpolation (daha stabil)
        float xm1 = (idx0 > 0) ? original[idx0 - 1] : original[idx0];
        float x0 = original[idx0];
        float x1 = original[idx0 + 1];
        float x2 = (idx0 + 2 < sampleCount) ? original[idx0 + 2] : original[idx0 + 1];
        
        float c0 = x0;
        float c1 = 0.5f * (x1 - xm1);
        float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
        float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
        
        buffer[i] = ((c3 * frac + c2) * frac + c1) * frac + c0;
    }
}

void PSOLA::applyAntiAliasFilter(std::vector<float>& buffer, int sampleCount, 
                                 int sampleRate, float pitchFactor)
{
    if (pitchFactor > 1.0f) {
        // Pitch yükseltiliyorsa, low-pass filter uygula
        float nyquist = sampleRate / 2.0f;
        float cutoff = nyquist / pitchFactor * 0.9f; // Güvenlik marjı
        
        applySimpleLowPass(buffer, sampleCount, sampleRate, cutoff);
    }
}

void PSOLA::applySimpleLowPass(std::vector<float>& buffer, int sampleCount,
                               int sampleRate, float cutoff)
{
    if (cutoff <= 0.0f || cutoff >= sampleRate/2.0f) return;
    
    // Simple one-pole low-pass filter
    float dt = 1.0f / sampleRate;
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float alpha = dt / (rc + dt);
    
    float prev = buffer[0];
    
    for (int i = 0; i < sampleCount; i++) {
        float x = buffer[i];
        float y = prev + alpha * (x - prev);
        buffer[i] = y;
        prev = y;
    }
}

void PSOLA::applyMinimalEffect(std::vector<float>& buffer, EffectType effect,
                              float, int)
{
    // Çok minimal efekt işlemleri
    switch (effect) {
        case BANANA: {
            // Sadece hafif ton ayarı
            for (int i = 0; i < (int)buffer.size(); i++) {
                buffer[i] *= 1.05f; // Hafif gain
            }
            break;
        }
        case DEVIL: {
            // Çok hafif distortion
            for (int i = 0; i < (int)buffer.size(); i++) {
                float x = buffer[i];
                if (x > 0.3f) x = 0.3f + (x - 0.3f) * 0.7f;
                if (x < -0.3f) x = -0.3f + (x + 0.3f) * 0.7f;
                buffer[i] = x;
            }
            break;
        }
        case FEMALE: {
            // Hafif high-pass
            float prev = buffer[0];
            for (int i = 1; i < (int)buffer.size(); i++) {
                float x = buffer[i];
                float y = x - prev * 0.1f;
                prev = x;
                buffer[i] = y;
            }
            break;
        }
        case ROBOT:
        case COMBINE:
        case EKO:
            // Bu efektler için minimal işlem yok
            break;
    }
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

void PSOLA::applyMinimalFiltering(std::vector<float>& buffer, EffectType effect,
                                 float strength, int sampleRate)
{
    if (strength <= 0.0f) return;
    
    switch (effect) {
        case BANANA: {
            // Banana: Çok hafif high-pass + hafif formant
            float formantFreq = 800.0f;
            
            // Apply very light high-pass
            for (int i = 1; i < (int)buffer.size(); i++) {
                float alpha = 0.01f * strength;
                buffer[i] = buffer[i] - buffer[i-1] * (1.0f - alpha);
            }
            
            // Ultra light formant enhancement
            static float phase = 0.0f;
            float phaseInc = 2.0f * M_PI * formantFreq / sampleRate;
            
            for (int i = 0; i < (int)buffer.size(); i++) {
                float formant = 1.0f + 0.1f * strength * sin(phase);
                buffer[i] *= formant;
                phase += phaseInc;
                if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
            }
            break;
        }
        
        case DEVIL: {
            // Devil: Minimal distortion + pitch variation
            float distortion = 0.15f * strength;
            
            for (int i = 0; i < (int)buffer.size(); i++) {
                float sample = buffer[i];
                
                // Very subtle distortion
                if (sample > 0) {
                    sample = 1.0f - expf(-sample * (1.0f + distortion));
                } else {
                    sample = -1.0f + expf(sample * (1.0f + distortion));
                }
                
                buffer[i] = buffer[i] * (1.0f - distortion) + sample * distortion;
            }
            break;
        }
        
        case FEMALE: {
            // Female: Clean pitch shift only, minimal formant
            for (int i = 1; i < (int)buffer.size(); i++) {
                // Very subtle high-pass
                float alpha = 0.02f * strength;
                buffer[i] = buffer[i] * (1.0f - alpha) + (buffer[i] - buffer[i-1]) * alpha;
            }
            break;
        }
        
        default:
            break;
    }
}

void PSOLA::applyRobotEffect(std::vector<float>& buffer, int sampleCount, 
                            int sampleRate, float pitchFactor)
{
    // Robot için daha agresif işlemler
    if (pitchFactor != 1.0f) {
        applyPitchShift(buffer, sampleCount, pitchFactor);
    }
    
    // Autotune uygula
    applyAutotune(buffer, sampleCount, sampleRate);
    
    // Light formant filtering
    applyRobotFormant(buffer, sampleCount, sampleRate);
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

void PSOLA::applyCaveEcho(std::vector<float>& buffer, int sampleCount, int sampleRate)
{
    // Echo için cleaner ayarlar
    float delayMs1 = 250.0f;    // Kısa delay
    float delayMs2 = 400.0f;    // Orta delay
    float feedback1 = 0.5f;     // Düşük feedback
    float feedback2 = 0.4f;
    float mix = 0.6f;          // Daha az mix

    int delaySamples1 = static_cast<int>(delayMs1 * sampleRate / 1000.0f);
    int delaySamples2 = static_cast<int>(delayMs2 * sampleRate / 1000.0f);

    if (delaySamples1 <= 0 || delaySamples2 <= 0)
        return;

    if ((int)m_echoBuffer1.size() != delaySamples1)
        m_echoBuffer1.assign(delaySamples1, 0.0f);
    if ((int)m_echoBuffer2.size() != delaySamples2)
        m_echoBuffer2.assign(delaySamples2, 0.0f);

    for (int i = 0; i < sampleCount; i++) {
        float dry = buffer[i];
        float echo1 = m_echoBuffer1[m_echoIndex1];
        float echo2 = m_echoBuffer2[m_echoIndex2];

        float totalEcho = echo1 * 0.6f + echo2 * 0.4f;
        buffer[i] = dry + totalEcho * mix;

        m_echoBuffer1[m_echoIndex1] = dry + echo1 * feedback1;
        m_echoBuffer2[m_echoIndex2] = dry + echo2 * feedback2;

        m_echoIndex1 = (m_echoIndex1 + 1) % delaySamples1;
        m_echoIndex2 = (m_echoIndex2 + 1) % delaySamples2;
    }
}

float PSOLA::hanningWindow(int n, int N)
{
    return 0.5f * (1.0f - cosf(2.0f * M_PI * n / (N - 1)));
}

void PSOLA::applyDCBlocker(std::vector<float>& buffer)
{
    static float x1 = 0.0f;
    static float y1 = 0.0f;
    const float R = 0.995f; // 0.99–0.998 arası ideal
    
    for (size_t i = 0; i < buffer.size(); i++) {
        float x = buffer[i];
        float y = x - x1 + R * y1;
        buffer[i] = y;
        x1 = x;
        y1 = y;
    }
}

void PSOLA::applyNoiseGate(std::vector<float>& buffer, float threshold)
{
    for (float& x : buffer) {
        if (fabs(x) < threshold)
            x = 0.0f;
    }
}

