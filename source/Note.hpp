#pragma once

#include <vector>
#include <string>

struct Note {
    std::string name;
    float freq;
};

inline std::vector<Note> GenerateChromaticNotes() {
    std::vector<Note> chromaticNotes;
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    for (int octave = 0; octave <= 8; ++octave) {
        for (int i = 0; i < 12; ++i) {
            float freq = 440.0f * powf(2.0f, ((octave * 12 + i - 57) / 12.0f)); // A4 = MIDI 69
            chromaticNotes.push_back({ names[i] + std::to_string(octave), freq });
        }
    }
    return chromaticNotes;
}

inline const std::vector<Note>& GetChromaticNotes() {
    static std::vector<Note> notes = GenerateChromaticNotes();
    return notes;
}
