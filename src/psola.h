#ifndef PSOLA_H
#define PSOLA_H

#include <vector>
#include <cstdint>

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
                 float filterStrength = 0.0f,
                 bool enableSmoothing = true);
    
    // Get default parameters for each effect type
    static float getDefaultPitchFactor(EffectType effect);
    static float getDefaultFilterStrength(EffectType effect);
    
private:
    // Core processing functions
    void applyPitchShift(std::vector<float>& buffer, int sampleCount, float pitchFactor);
    void applyRobotEffect(std::vector<float>& buffer, int sampleCount, int sampleRate, float pitchFactor);
    void applyAutotune(std::vector<float>& buffer, int sampleCount, int sampleRate);
    float estimatePitchFrequency(const std::vector<float>& frame, int sampleRate);
    void applyRobotFormant(std::vector<float>& buffer, int sampleCount, int sampleRate);
    
    // Filtering functions
    void applyInputHighPass(std::vector<float>& buffer, int sampleRate, float cutoff);
    void applyEffectFilter(std::vector<float>& buffer, EffectType effect, 
                         float strength, int sampleRate);
    void applyHighPassFilter(std::vector<float>& buffer, float strength, 
                           float cutoff, int sampleRate);
    void applyLowPassFilter(std::vector<float>& buffer, float strength,
                          float cutoff, int sampleRate);
    void applyOutputLimiting(std::vector<float>& buffer, int sampleCount, EffectType effect, bool hasSignal);
    
    // Effect enhancement functions
    void applyRingModulation(std::vector<float>& buffer, int sampleCount, int sampleRate, float modFreq);
    void applyVibrato(std::vector<float>& buffer, int sampleCount, int sampleRate, 
                      float rate, float depth);
    void applyCaveEcho(std::vector<float>& buffer, int sampleCount, int sampleRate);
    
    // Utility functions
    float hanningWindow(int n, int N);
    
    // Member variables for continuity
    float m_lastSample;
    float m_hpPrev;
    float m_lastInput;
    float m_ringPhase = 0.0f;
    float m_vibratoPhase = 0.0f;
    
    // Constants
    static constexpr int MIN_PITCH_HZ = 80;
    static constexpr int MAX_PITCH_HZ = 400;
};

#endif // PSOLA_H
