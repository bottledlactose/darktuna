#pragma once

#include <string>
#include <unordered_map>
#include <map>

#include "SDL3/SDL_events.h"
#include "portaudio.h"
#include "Note.hpp"

// Forward declarations
struct SDL_Window;
struct SDL_Renderer;

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define BUFFER_SIZE 2048

struct App {
private:
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;

    // Current audio device index
    int mCurrentAudioDeviceIndex = -1;
    // Name of the currently selected audio API
    std::string mHostApiName;
    // Mapping audio devices to their host API and device index
    std::unordered_map<std::string, std::map<int, std::string>> mAudioDevices;

    // Audio stream
    float mAudioBuffer[BUFFER_SIZE];
    int mBufferIndex = 0;
    bool mIsReadyForProcessing = false;
    PaStream *mStream = nullptr;

    // Audio state
    float mDetectedFrequency = 0.0f;
    const Note* mCurrentNote = nullptr;
    float mCentsOff = 0.0f;
    float mSignalStrength = 0.0f;

    // UI state
    bool mShowAboutMenu = false;
    bool mShowSettingsMenu = false;

    App() = default;
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    static int AudioCallback(const void *input, void *, unsigned long frames,
        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *);
    void StartAudioStream(int deviceIndex);

public:
    static App& Get() {
        static App instance;
        return instance;
    }

    bool Initialize();
    void Shutdown();
    void ProcessEvent(SDL_Event *event);
    
    void BeginFrame();
    void Update();
    void Draw();
    void EndFrame();

    void UpdateAudioDevices();

    inline SDL_Window *GetWindow() const {
        return mWindow;
    }

    inline SDL_Renderer *GetRenderer() const {
        return mRenderer;
    }
};
