#ifndef PSOLA_H
#define PSOLA_H

#include <vector>
#include <cstdint>
#include <cmath>
class PSOLA
{
public:
    // Effect types with different parameters
    enum EffectType {
        BANANA,     // Baby voice - high pitch, gentle
        ROBOT,      // Robot voice - moderate pitch, mechanical
        DEVIL,      // Devil voice - low pitch, deep
        FEMALE,     // Female voice - moderate-high pitch
        COMBINE,    // Combined effect - custom parameters
        EKO         // Echo effect - subtle pitch shift
    };

    PSOLA();
    ~PSOLA();
    
    // Main PSOLA processing function with configurable parameters
    void process(int16_t* pcm, int sampleCount, int sampleRate, 
                 float pitchFactor = 1.0f, 
                 EffectType effect = BANANA,
                 float filterStrength = 0.0f);
    
    // Get default parameters for each effect type
    static float getDefaultPitchFactor(EffectType effect);
    static float getDefaultFilterStrength(EffectType effect);
    
private:
    // Core processing functions
    void applyPitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor);
    void applyCleanPitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor, EffectType);
    void applySimplePitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor);
    void applyUltraCleanPitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor);
    void applyStrongAntiAliasFilter(std::vector<float>& buffer, int sampleCount, float);
    void applyPureEchoOnly(std::vector<float>& buffer, int sampleCount, int sampleRate);
    void applyGentleLowPass(std::vector<float>& buffer, int sampleCount, float);
    void applyRobotEffect(std::vector<float>& buffer, int sampleCount, int sampleRate, float pitchFactor);
    void applyAutotune(std::vector<float>& buffer, int sampleCount, int sampleRate);
    float estimatePitchFrequency(const std::vector<float>& frame, int sampleRate);
    void applyRobotFormant(std::vector<float>& buffer, int sampleCount, int sampleRate);
    
    // Filtering functions
    void applyMinimalFiltering(std::vector<float>& buffer, EffectType effect,
                             float strength, int sampleRate);
    void applyAntiAliasFilter(std::vector<float>& buffer, int sampleCount, 
                             int sampleRate, float pitchFactor);
    void applySimpleLowPass(std::vector<float>& buffer, int sampleCount,
                           int sampleRate, float cutoff);
    void applyMinimalEffect(std::vector<float>& buffer, EffectType effect, float, int);
    
    // Effect enhancement functions
    void applyCaveEcho(std::vector<float>& buffer, int sampleCount, int sampleRate);
    
    // Utility functions
    float hanningWindow(int n, int N);
    
    // NEW: Anti-popping helper functions
    void normalizeRMS(std::vector<float>& buffer, float targetRMS = 0.18f);
    void applyEdgeFade(std::vector<float>& buffer, int fadeSamples = 32);
    
    // NEW: Deep noise elimination functions
    void applyDCBlocker(std::vector<float>& buffer);
    void applyNoiseGate(std::vector<float>& buffer, float threshold = 0.015f);
    
    
    // Member variables for continuity
    float m_lastSample;
    
    // Cave echo effect variables
    std::vector<float> m_echoBuffer1;
    std::vector<float> m_echoBuffer2;
    int m_echoIndex1 = 0;
    int m_echoIndex2 = 0;
    
    // Constants
    static constexpr int MIN_PITCH_HZ = 80;
    static constexpr int MAX_PITCH_HZ = 400;
};

// NEW: Inline helper functions for anti-popping
inline void PSOLA::normalizeRMS(std::vector<float>& buffer, float targetRMS)
{
    double sum = 0.0;
    for (float x : buffer)
        sum += x * x;

    float rms = sqrt(sum / buffer.size());
    
    // KRİTİK: sessizse normalize etme
    if (rms < 0.02f) return;

    float gain = targetRMS / rms;
    gain = std::min(gain, 3.0f); // gain clamp

    for (float& x : buffer)
        x *= gain;
}

inline void PSOLA::applyEdgeFade(std::vector<float>& buffer, int fadeSamples)
{
    int bufSize = buffer.size();
    if (bufSize <= fadeSamples * 2) return;
    
    for (int i = 0; i < fadeSamples; i++) {
        float g = (float)i / fadeSamples;
        buffer[i] *= g;
        buffer[bufSize - 1 - i] *= g;
    }
}

#endif // PSOLA_H
