#pragma once

#include "Note.hpp"

namespace Tuner {

float DetectFrequencyAutocorrelation(const float *buffer, int size, float sample_rate);
const Note& GetClosestNote(float freq);
float GetCentsOff(float freq, float refFreq);

} // namespace Tuner
