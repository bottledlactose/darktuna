#include "App.hpp"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include "portaudio.h"

#include "Tuner.hpp"

int App::AudioCallback(const void *input, void *, unsigned long frames,
        const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *) {

    App &instance = App::Get();
    const float *in = (const float *)input;
        for (unsigned long i = 0; i < frames; ++i) {
        instance.mAudioBuffer[instance.mBufferIndex++] = *in++;
        if (instance.mBufferIndex >= BUFFER_SIZE) {
            instance.mBufferIndex = 0;
            instance.mIsReadyForProcessing = true;
        }
    }
    return paContinue;
}

void App::StartAudioStream(int deviceIndex) {
    if (mStream) {
        Pa_StopStream(mStream);
        Pa_CloseStream(mStream);
        mStream = nullptr;
    }

    PaStreamParameters inputParams;
    inputParams.device = deviceIndex;
    inputParams.channelCount = 1;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(deviceIndex)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;

    Pa_OpenStream(&mStream, &inputParams, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, AudioCallback, nullptr);
    Pa_StartStream(mStream);
}

bool App::Initialize() {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    float display_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    mWindow = SDL_CreateWindow("Darktune", 800, 300, window_flags);
    if (!mWindow) {
        SDL_Log("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(mWindow, nullptr);
    if (!mRenderer) {
        SDL_Log("Failed to create SDL renderer: %s", SDL_GetError());
        return false;
    }

    SDL_SetRenderVSync(mRenderer, 1);
    SDL_ShowWindow(mWindow);

    // Set up ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    // Set up ImGui scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(display_scale);
    style.FontScaleDpi = display_scale;

    ImGui_ImplSDL3_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer3_Init(mRenderer);

    // Initialize PortAudio
    Pa_Initialize();
    UpdateAudioDevices();

    // TODO: Load basic config file for settings
    if (!mAudioDevices.empty()) {
        mHostApiName = mAudioDevices.begin()->first;
    }

    return true;
}

void App::Shutdown() {
    if (mStream) {
        Pa_StopStream(mStream);
        Pa_CloseStream(mStream);
    }
    Pa_Terminate();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void App::BeginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void App::Update() {
    if (mIsReadyForProcessing) {
        // Calculate signal strength
        mSignalStrength = 0.0f;

        for (int i = 0; i < BUFFER_SIZE; ++i) {
            mSignalStrength += mAudioBuffer[i] * mAudioBuffer[i];
        }
        mSignalStrength = sqrtf(mSignalStrength / BUFFER_SIZE);

        // TODO: Make signal strength configurable
        if (mSignalStrength > 0.0065f) {
            mDetectedFrequency = Tuner::DetectFrequencyAutocorrelation(mAudioBuffer, BUFFER_SIZE, SAMPLE_RATE);

            if (mDetectedFrequency > 20.0f && mDetectedFrequency < 500.0f) {
                mCurrentNote = &Tuner::GetClosestNote(mDetectedFrequency);
                mCentsOff = Tuner::GetCentsOff(mDetectedFrequency, mCurrentNote->freq);
            }
        }

        mIsReadyForProcessing = false;
    }
}

void App::Draw() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Darktune\nA guitar tuner using ImGui, SDL3, and PortAudio.");
        ImGui::Separator();
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (mShowAboutMenu) {
        ImGui::OpenPopup("About");
        mShowAboutMenu = false;
    }

    if (ImGui::BeginPopupModal("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (ImGui::BeginCombo("Host API", mHostApiName.c_str())) {
            for (auto hostApi : mAudioDevices) {
                bool isSelected = hostApi.first == mHostApiName;
                if (ImGui::Selectable(hostApi.first.c_str(), isSelected)) {
                    mHostApiName = hostApi.first;
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        if (ImGui::Button("OK")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (mShowSettingsMenu) {
        ImGui::OpenPopup("Settings");
        mShowSettingsMenu = false;
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem("Settings")) {
                mShowSettingsMenu = true;
            }

            if (ImGui::MenuItem("Exit")) {
                SDL_Event quit_event = { .type = SDL_EVENT_QUIT };
                SDL_PushEvent(&quit_event);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Devices")) {
            for (auto pair : mAudioDevices[mHostApiName]) {
                bool isSelected = (mCurrentAudioDeviceIndex == pair.first);
                if (ImGui::MenuItem((pair.second + "##" + std::to_string(pair.first)).c_str(), nullptr, isSelected)) {
                    mCurrentAudioDeviceIndex = pair.first;
                    StartAudioStream(mCurrentAudioDeviceIndex);
                }
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                mShowAboutMenu = true;
            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
    ImVec2 windowSize = ImVec2(
        io.DisplaySize.x,
        io.DisplaySize.y - ImGui::GetFrameHeight()
    );
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    ImGui::Begin("MainContent", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoCollapse);

    // Show RMS value
    ImGui::Text("Strength (RMS): %.6f\n", mSignalStrength);

    if (mCurrentNote) {
        ImGui::Text("Detected: %2.f Hz", mDetectedFrequency);
        ImGui::Text("Note: %s (%.2f Hz)", mCurrentNote->name, mCurrentNote->freq);
        ImGui::Text("Cents off: %.2f", mCentsOff);

        float needlePos = (mCentsOff + 50.0f) / 100.0f;
        needlePos = fminf(fmaxf(needlePos, 0.0f), 1.0f);
        ImGui::Text("Tuning");
        ImGui::ProgressBar(needlePos, ImVec2(300, 20));

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20)); // More vertical spacing
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255)); // White text

        // Scale text manually (like an H1 heading)
        ImGui::SetWindowFontScale(3.5f); // H1-style scaling
        ImVec2 textSize = ImGui::CalcTextSize(mCurrentNote->name);
        float textX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;

        ImGui::SetCursorPosX(textX);
        ImGui::Text("%s", mCurrentNote->name);

        ImGui::SetWindowFontScale(1.0f); // Reset scale
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

    } else {
        ImGui::Text("Listening...");
    }

    ImGui::End();



    //ImGui::OpenPopup("AboutPopup");
}

void App::EndFrame() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::Render();
    SDL_SetRenderScale(mRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(mRenderer, 0.0f, 0.0f, 0.0f, 1.0f);
    SDL_RenderClear(mRenderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), mRenderer);
    SDL_RenderPresent(mRenderer);
}

void App::UpdateAudioDevices() {
    int numDevices = Pa_GetDeviceCount();
    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {
            const PaHostApiInfo *hostApi = Pa_GetHostApiInfo(info->hostApi);

            if (hostApi) {
                std::string hostApiName = hostApi->name;
                std::string deviceName = info->name;

                mAudioDevices[hostApiName][i] = deviceName;
            }
        }
    }
}
