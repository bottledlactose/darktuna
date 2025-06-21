#include "Tuner.hpp"

#include <cmath>

float Tuner::DetectFrequencyAutocorrelation(const float *buffer, int size, float sample_rate) {
    int best_lag = 0;
    float max_correlation = 0.0f;

    for (int lag = 20; lag < size / 2; ++lag) {
        float sum = 0.0f;
        for (int i = 0; i < size - lag; ++i) {
            sum += buffer[i] * buffer[i + lag];
        }
        if (sum > max_correlation) {
            max_correlation = sum;
            best_lag = lag;
        }
    }

    if (best_lag == 0) return 0.0f;
    return sample_rate / best_lag;
}

const Note& Tuner::GetClosestNote(float freq) {
    const auto& notes = GetChromaticNotes();
    const Note* closest = &notes[0];
    float minDiff = fabsf(freq - closest->freq);
    
    for (const auto& note : notes) {
        float diff = fabsf(freq - note.freq);
        if (diff < minDiff) {
            minDiff = diff;
            closest = &note;
        }
    }
    return *closest;
}

float Tuner::GetCentsOff(float freq, float refFreq) {
    return 1200.0f * log2f(freq / refFreq);
}
