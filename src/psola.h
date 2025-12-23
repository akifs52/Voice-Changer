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
    
    // Individual PSOLA functions (can be called separately if needed)
    int estimatePitchPeriod(const std::vector<float>& x, int sampleRate);
    std::vector<int> computePitchMarks(int pitchPeriod, int signalLength);
    
    // Get default parameters for each effect type
    static float getDefaultPitchFactor(EffectType effect);
    static float getDefaultFilterStrength(EffectType effect);
    
private:
    // Helper functions
    float hann(int n, int N);
    void applyHighPassFilter(std::vector<float>& buffer, float strength);
    void applySmoothing(std::vector<float>& buffer, int windowSize);
    
    // Constants
    static constexpr int MIN_PITCH_HZ = 80;
    static constexpr int MAX_PITCH_HZ = 400;
};

#endif // PSOLA_H
