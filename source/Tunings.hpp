#pragma once
#include <vector>
#include <string>

inline const std::vector<std::pair<std::string, std::vector<std::string>>> kGuitarTunings = {
    {"Standard E",  {"E2", "A2", "D3", "G3", "B3", "E4"}},
    {"Drop D",      {"D2", "A2", "D3", "G3", "B3", "E4"}},
    {"DADGAD",      {"D2", "A2", "D3", "G3", "A3", "D4"}},
    {"Half Step Down", {"Eb2", "Ab2", "Db3", "Gb3", "Bb3", "Eb4"}},
    {"Full Step Down", {"D2", "G2", "C3", "F3", "A3", "D4"}},
    {"Open D",      {"D2", "A2", "D3", "F#3", "A3", "D4"}},
    {"Open E",      {"E2", "B2", "E3", "G#3", "B3", "E4"}},
    {"Open G",      {"D2", "G2", "D3", "G3", "B3", "D4"}},
    {"Open A",      {"E2", "A2", "E3", "A3", "C#4", "E4"}},
    {"Double Drop D", {"D2", "A2", "D3", "G3", "B3", "D4"}},
    {"Drop C",      {"C2", "G2", "C3", "F3", "A3", "D4"}},
    {"Drop B",      {"B1", "F#2", "B2", "E3", "G#3", "C#4"}},
    {"Drop A",      {"A1", "E2", "A2", "D3", "F#3", "B3"}},
    {"Open C",      {"C2", "G2", "C3", "G3", "C4", "E4"}},
    {"Open B",      {"B1", "F#2", "B2", "F#3", "B3", "D#4"}},
    {"Open A Minor",{"E2", "A2", "E3", "A3", "C4", "E4"}},
    {"C6 Tuning",   {"C2", "A2", "C3", "G3", "C4", "E4"}},
    {"All Fourths", {"E2", "A2", "D3", "G3", "C4", "F4"}},
    {"Major Thirds",{"C2", "E2", "G#2", "C3", "E3", "G#3"}},
    {"Nashville",   {"E3", "A3", "D4", "G4", "B3", "E4"}}
};
