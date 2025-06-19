#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"

#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"

#include "portaudio.h"

// TODO: Maybe make these configurable?
#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512
#define BUFFER_SIZE 2048

// These are populated on initialization
static std::vector<std::string> g_deviceNames;
static std::vector<int> g_deviceIndices;

// Audio context
static float g_audioBuffer[BUFFER_SIZE];
static int g_bufferIndex = 0;
static bool g_readyForProcessing = false;
static PaStream *g_stream = nullptr;
static int g_selectedDevice = -1;

// Note definitions
struct Note {
    const char* name;
    float freq;
};

const Note notes[] = {
    {"E2", 82.41}, {"A2", 110.00}, {"D3", 146.83},
    {"G3", 196.00}, {"B3", 246.94}, {"E4", 329.63},
};

// Audio state
static float g_detectedFreq = 0.0f;
static const Note* g_currentNote = nullptr;
static float g_cents = 0.0f;
static float g_rms = 0.0f;

// SDL context
static SDL_Window *g_window;
static SDL_Renderer *g_renderer;

float DetectFrequencyAutocorrelation(const float *buffer, int size, float sample_rate) {
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

const Note& GetClosestNote(float freq) {
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

float GetCentsOff(float freq, float refFreq) {
    return 1200.0f * log2f(freq / refFreq);
}

static int AudioCallback(const void *input, void *, unsigned long frames,
        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *) {
    const float *in = (const float *)input;
        for (unsigned long i = 0; i < frames; ++i) {
        g_audioBuffer[g_bufferIndex++] = *in++;
        if (g_bufferIndex >= BUFFER_SIZE) {
            g_bufferIndex = 0;
            g_readyForProcessing = true;
        }
    }
    return paContinue;
}

void StartStream(int device_index) {
    if (g_stream) {
        Pa_StopStream(g_stream);
        Pa_CloseStream(g_stream);
        g_stream = nullptr;
    }

    PaStreamParameters inputParams;
    inputParams.device = device_index;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(device_index)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    Pa_OpenStream(&g_stream, &inputParams, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, AudioCallback, nullptr);
    Pa_StartStream(g_stream);
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    g_window = SDL_CreateWindow("Darktune", 800, 600, window_flags);

    if (!g_window) {
        fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    g_renderer = SDL_CreateRenderer(g_window, nullptr);

    if (!g_renderer) {
        fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderVSync(g_renderer, 1);
    SDL_ShowWindow(g_window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // TODO: Enable gamepad support?

    ImGui::StyleColorsDark();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
    ImGui_ImplSDLRenderer3_Init(g_renderer);

    // Initialize audio library
    Pa_Initialize();

    int num_device = Pa_GetDeviceCount();
    for (int i = 0; i < num_device; ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {

            const PaHostApiInfo *host_api = Pa_GetHostApiInfo(info->hostApi);

            std::string label = std::string(info->name) + " (" + host_api->name + ")";
            g_deviceNames.emplace_back(label);


            //g_deviceNames.emplace_back(info->name + );
            g_deviceIndices.push_back(i);
        }
    }

    // Select the first device by default
    if (!g_deviceIndices.empty()) {
        g_selectedDevice = g_deviceIndices[0];
        StartStream(g_selectedDevice);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {

    SDL_Delay(10);

    // Start the ImGui frame
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();


    if (g_readyForProcessing) {

        // Calculate signal power (the hacky way)
        float rms = 0.0f;
        for (int i = 0; i < BUFFER_SIZE; ++i)
            rms += g_audioBuffer[i] * g_audioBuffer[i];
        rms = sqrtf(rms / BUFFER_SIZE);

        if (rms > 0.01f) {
            g_detectedFreq = DetectFrequencyAutocorrelation(g_audioBuffer, BUFFER_SIZE, SAMPLE_RATE);

            // Only listen to guitar noise
            if (g_detectedFreq < 60.0f || g_detectedFreq > 500.0f) {
                g_detectedFreq = 0.0f; // TODO: Skip UI update instead
            }

            if (g_detectedFreq > 20.0f) {
                g_currentNote = &GetClosestNote(g_detectedFreq);
                g_cents = GetCentsOff(g_detectedFreq, g_currentNote->freq);
            }
        }

        g_readyForProcessing = false;
    }

    ImGui::Begin("Tuner");

    if (g_currentNote) {
        ImGui::Text("Detected: %2.f Hz", g_detectedFreq);
        ImGui::Text("Note: %s (%.2f Hz)", g_currentNote->name, g_currentNote->freq);
        ImGui::Text("Cents off: %.2f", g_cents);

        float needlePos = (g_cents + 50.0f) / 100.0f;
        needlePos = fminf(fmaxf(needlePos, 0.0f), 1.0f);
        ImGui::Text("Tuning");
        ImGui::ProgressBar(needlePos, ImVec2(300, 20));
    } else {
        ImGui::Text("Listening...");
    }

    ImGui::End();

    // Create a testing window
    ImGui::Begin("Devices");

    for (int i = 0; i < g_deviceNames.size(); ++i) {
        bool is_selected = (g_selectedDevice == g_deviceIndices[i]);
        if (ImGui::Selectable((g_deviceNames[i] + "##" + std::to_string(i)).c_str(), is_selected)) {
            g_selectedDevice = g_deviceIndices[i];
            StartStream(g_selectedDevice);
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    SDL_SetRenderScale(g_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(g_renderer, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderClear(g_renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
    SDL_RenderPresent(g_renderer);

    return SDL_APP_CONTINUE;    
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);

    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}
