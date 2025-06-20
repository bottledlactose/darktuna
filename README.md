<p align="center">
  <img src="https://raw.githubusercontent.com/bottledlactose/darktuna/refs/heads/trunk/images/logo.png" alt="Darktuna logo" width="250">
</p>

# Darktuna

A high-precision, real-time guitar tuner built with C++, SDL3, ImGui and PortAudio.
It analyzes audio input to detect frequency, estimate tuning accuracy, and guide you visually with intuitive feedback.

![Darktuna screenshot](https://raw.githubusercontent.com/bottledlactose/darktuna/refs/heads/trunk/images/screenshot.png)

## Features

- Real-time Frequency Detection
    - Uses autocorrelation to detect fundamental frequency from audio input
    - Accurately identifies notes within a 20 Hz - 500 Hz range, ideal for guitar and other string instruments
- Tuning Accuracy
    - Calculates how many **cents** you're off from the nearest note
    - Displays tuning guidance:
        - Green when you're in tune
        - Orange with "Tune down" if you're sharp
        - Orange with "Tune up" if you're flat
- Visual Feedback
    - Displays:
        - Signal strength (RMS)
        - Detected frequency in Hz
        - Closest musical note
        - Cents deviation
    - Clear, dark-themed interface built with ImGui for an elegant and responsive UI.
- Adjustable settings
    - RMS Threshold: Filter out background noise
    - Cents Tolerance: Customize how strict "in tune" detection should be
- Device Selection
    - Supports multiple audio devices and audio APIs
    - Allows switching input devices on the fly

## Built With

- C++17
- SDL3
- Dear ImGui
- PortAudio
