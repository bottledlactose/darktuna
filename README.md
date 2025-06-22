<p align="center">
  <img src="https://raw.githubusercontent.com/bottledlactose/darktuna/refs/heads/develop/images/logo-128.png" alt="Darktune logo" width="128">
</p>

# Darktuna

**Darktuna** is a real-time, high-precision guitar tuner built with C++, SDL3, ImGui, and PortAudio.

It analyzes microphone input to detect pitch and visually guides you toward accurate tuning—whether you're using standard tuning or experimenting with open tunings.

![Darktuna screenshot](https://raw.githubusercontent.com/bottledlactose/darktuna/refs/heads/trunk/images/screenshot.png)

---

## Features

- **Accurate Pitch Detection**
  - Uses autocorrelation to estimate the fundamental frequency
  - Optimized for guitar and other stringed instruments (20 Hz – 500 Hz)

- **Tuning Feedback**
  - Identifies the closest musical note
  - Displays cents deviation from the target note
  - Clear visual guidance:
    - Green: In tune
    - Orange: Tune down (sharp)
    - Orange: Tune up (flat)

- **Visual Indicators**
  - Real-time display of:
    - Signal strength (RMS)
    - Detected frequency (Hz)
    - Closest note
    - Cents offset
  - Dark-themed, responsive interface built with ImGui

- **Adjustable Settings**
  - RMS Threshold: filter out background noise
  - Cents Tolerance: customize how strict the tuning guidance should be

- **Audio Input Selection**
  - Choose from multiple input devices and audio APIs
  - Switch devices on the fly during use

- **Built-In Tunings**
  - Includes over 20 popular tuning presets such as:
    - Standard E
    - Drop D
    - DADGAD
    - Open G, Open D, and more
  - Visual reference for each tuning

---

## Installation

### Requirements

Before using or building Darktuna, ensure:

- Your system allows **microphone access**
- You have an audio input device (microphone or interface)

#### Windows

- Install the **Visual C++ Runtime** if it's not already present:  
  [Download from Microsoft](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)

---

### Building from Source

Darktuna uses CMake and requires the following dependencies:

- CMake 3.18 or later
- C++17-compatible compiler (MSVC, Clang, or GCC)
- SDL3
- Dear ImGui
- PortAudio

```bash
git clone https://github.com/bottledlactose/darktuna.git
cd darktuna
mkdir build && cd build
cmake ..
cmake --build .
```

> If needed, you can use package managers like vcpkg or conan to install SDL3 and PortAudio.

---

## Usage

1. Connect your microphone or audio interface.
2. Launch the application.
3. Select your input device from the "Devices" menu.
4. Pluck a string and observe the tuning feedback in real time.
5. Open the "Settings" menu to fine-tune sensitivity and tolerance.

---

## License

This project is licensed under the MIT License.
Feel free to use, modify, and distribute it as needed.
