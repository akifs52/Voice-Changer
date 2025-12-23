#include "psola.h"
#include <cmath>
#include <algorithm>
#include <Qt>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

PSOLA::PSOLA()
{
}

PSOLA::~PSOLA()
{
}

float PSOLA::getDefaultPitchFactor(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 1.6f;    // High pitch for baby voice
        case ROBOT:    return 0.8f;    // Lower pitch for robot
        case DEVIL:    return 0.6f;    // Very low pitch for devil
        case FEMALE:   return 1.3f;    // Higher pitch for female voice
        case COMBINE:  return 1.2f;    // Moderate pitch for combined
        case EKO:      return 1.1f;    // Slight pitch shift for echo
        default:       return 1.0f;
    }
}

float PSOLA::getDefaultFilterStrength(EffectType effect)
{
    switch (effect) {
        case BANANA:   return 0.3f;    // Gentle high-pass for baby voice
        case ROBOT:    return 0.1f;    // Minimal filtering for robot
        case DEVIL:    return 0.0f;    // No filtering for devil
        case FEMALE:   return 0.2f;    // Light filtering for female
        case COMBINE:  return 0.15f;   // Moderate filtering
        case EKO:      return 0.05f;   // Very light filtering
        default:       return 0.0f;
    }
}

void PSOLA::process(int16_t* pcm, int sampleCount, int sampleRate, 
                   float pitchFactor, EffectType effect,
                   float filterStrength, bool enableSmoothing)
{
    // Safety checks
    if (!pcm || sampleCount <= 0 || sampleRate <= 0) {
        return;
    }
    
    if (pitchFactor <= 0.1f || pitchFactor > 10.0f) {
        return; // Invalid pitch factor
    }
    
    // Minimum sample count for meaningful processing
    if (sampleCount < 100) {
        return;
    }

    // Use default parameters if not specified
    if (filterStrength < 0.0f) {
        filterStrength = getDefaultFilterStrength(effect);
    }

    // -----------------------------
    // Enhanced smooth pitch shifting
    // -----------------------------
    
    // Create temporary buffer for processing
    std::vector<float> tempBuffer(sampleCount + 4, 0.0f); // Extra padding for smooth edges
    
    // Convert input to float with gentle preprocessing
    for (int i = 0; i < sampleCount; ++i) {
        tempBuffer[i + 2] = pcm[i] / 32768.0f;
    }
    
    // Apply edge smoothing to prevent boundary clicks
    tempBuffer[0] = tempBuffer[2] * 0.5f;
    tempBuffer[1] = tempBuffer[2] * 0.8f;
    tempBuffer[sampleCount + 2] = tempBuffer[sampleCount + 1] * 0.8f;
    tempBuffer[sampleCount + 3] = tempBuffer[sampleCount + 1] * 0.5f;
    
    // Apply smoothing to reduce harsh transitions
    if (enableSmoothing) {
        applySmoothing(tempBuffer, 5); // Increased window size
    }
    
    // Calculate new sample count based on pitch factor
    int newSampleCount = static_cast<int>(sampleCount / pitchFactor);
    
    // Ensure reasonable bounds
    newSampleCount = qMax(50, qMin(newSampleCount, sampleCount * 2));
    
    // Create resampled buffer
    std::vector<float> resampled(newSampleCount + 4, 0.0f);
    
    // Enhanced resampling with better interpolation and overlap handling
    for (int i = 0; i < newSampleCount; ++i) {
        float srcIndex = i * pitchFactor + 2.0f; // Offset for padding
        int index1 = static_cast<int>(srcIndex);
        int index2 = qMin(index1 + 1, sampleCount + 3);
        
        if (index1 >= 0 && index1 < sampleCount + 4) {
            float weight = srcIndex - index1;
            
            // Improved cubic interpolation with better boundary handling
            if (index1 > 1 && index2 < sampleCount + 2) {
                float w2 = weight * weight;
                float w3 = w2 * weight;
                
                // Catmull-Rom spline coefficients for smoother interpolation
                float a0 = -0.5f * w3 + w2 - 0.5f * weight;
                float a1 = 1.5f * w3 - 2.5f * w2 + 1.0f;
                float a2 = -1.5f * w3 + 2.0f * w2 + 0.5f * weight;
                float a3 = 0.5f * w3 - 0.5f * w2;
                
                int index0 = qMax(0, index1 - 1);
                int index3 = qMin(sampleCount + 3, index2 + 1);
                
                resampled[i + 2] = a0 * tempBuffer[index0] + 
                                 a1 * tempBuffer[index1] + 
                                 a2 * tempBuffer[index2] + 
                                 a3 * tempBuffer[index3];
            } else {
                // Fallback to linear interpolation for boundaries
                resampled[i + 2] = (1.0f - weight) * tempBuffer[index1] + weight * tempBuffer[index2];
            }
        }
    }
    
    // Apply gentle filtering based on effect type
    if (filterStrength > 0.0f) {
        applyHighPassFilter(resampled, filterStrength * 0.7f); // Reduced filter strength
    }
    
    // Apply post-smoothing to reduce remaining artifacts
    if (enableSmoothing) {
        applySmoothing(resampled, 3);
    }
    
    // Copy back to original buffer with smooth transitions and overlap
    for (int i = 0; i < sampleCount; ++i) {
        float sample = 0.0f;
        
        if (i < newSampleCount) {
            sample = resampled[i + 2];
        } else {
            // Very smooth fade out at the end
            int remaining = sampleCount - i;
            if (remaining > 0 && newSampleCount > 0) {
                float fadeRatio = (float)remaining / sampleCount;
                float fadeWeight = 1.0f - (fadeRatio * fadeRatio); // Quadratic fade
                sample = resampled[newSampleCount + 1] * fadeWeight * 0.3f; // Reduced end amplitude
            }
        }
        
        // Gentle normalization with soft clipping
        if (sample > 0.9f) {
            sample = 0.9f + (sample - 0.9f) * 0.1f; // Soft clipping
        } else if (sample < -0.9f) {
            sample = -0.9f + (sample + 0.9f) * 0.1f; // Soft clipping
        }
        
        // Apply gentle low-pass to reduce high-frequency crackling
        if (i > 0) {
            sample = sample * 0.7f + (pcm[i-1] / 32768.0f) * 0.3f;
        }
        
        pcm[i] = static_cast<int16_t>(sample * 32767);
    }
}

void PSOLA::applyHighPassFilter(std::vector<float>& buffer, float strength)
{
    if (buffer.size() < 2 || strength <= 0.0f) return;
    
    std::vector<float> filtered = buffer;
    
    for (size_t i = 1; i < buffer.size(); ++i) {
        // Simple high-pass filter with adjustable strength
        filtered[i] = buffer[i] - strength * buffer[i-1];
        
        // Limit the filtering effect to prevent harsh transitions
        float diff = filtered[i] - buffer[i];
        if (std::abs(diff) > 0.1f) {
            filtered[i] = buffer[i] + (diff > 0 ? 0.1f : -0.1f);
        }
    }
    
    buffer = filtered;
}

void PSOLA::applySmoothing(std::vector<float>& buffer, int windowSize)
{
    if (buffer.size() < windowSize * 2) return;
    
    std::vector<float> smoothed = buffer;
    
    for (size_t i = windowSize; i < buffer.size() - windowSize; ++i) {
        float sum = 0.0f;
        for (int j = -windowSize; j <= windowSize; ++j) {
            sum += buffer[i + j];
        }
        smoothed[i] = sum / (2 * windowSize + 1);
    }
    
    buffer = smoothed;
}

int PSOLA::estimatePitchPeriod(const std::vector<float>& x, int sampleRate)
{
    int minLag = sampleRate / MAX_PITCH_HZ; // 400 Hz
    int maxLag = sampleRate / MIN_PITCH_HZ;  // 80 Hz

    float best = 0.0f;
    int bestLag = minLag;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        float sum = 0.0f;
        for (size_t i = 0; i + lag < x.size(); ++i)
            sum += x[i] * x[i + lag];

        if (sum > best) {
            best = sum;
            bestLag = lag;
        }
    }
    return bestLag;
}

std::vector<int> PSOLA::computePitchMarks(int pitchPeriod, int signalLength)
{
    std::vector<int> marks;
    for (int i = pitchPeriod; i < signalLength; i += pitchPeriod)
        marks.push_back(i);
    return marks;
}

float PSOLA::hann(int n, int N)
{
    return 0.5f * (1.0f - cosf(2.0f * M_PI * n / (N - 1)));
}
