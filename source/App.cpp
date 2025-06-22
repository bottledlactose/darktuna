#include "App.hpp"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_timer.h"
#include "SDL3/SDL_video.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include "portaudio.h"

#include "Tuner.hpp"
#include "Tunings.hpp"

#include "logo.h"

SDL_Surface* CreateSurfaceFromIcon() {
    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        LOGO_WIDTH,               // width
        LOGO_HEIGHT,              // height
        SDL_PIXELFORMAT_RGBA32,   // pixel format
        (void*)logo_data,         // pixel data
        LOGO_WIDTH * 4            // pitch (width * 4 bytes per pixel)
    );
    return surface;
}

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

void ApplyDarkboxTheme(ImGuiStyle& style) {
    ImVec4 orange = ImVec4(0.839f, 0.365f, 0.055f, 1.0f); // Gruvbox orange (#d65d0e)

    // Window and Popup Backgrounds
    style.Colors[ImGuiCol_WindowBg]   = ImVec4(0.06f, 0.06f, 0.06f, 1.0f);  // Main window background
    style.Colors[ImGuiCol_PopupBg]    = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);  // Popups, combo dropdowns

    // Menu Bar
    style.Colors[ImGuiCol_MenuBarBg] = orange;

    // Title Bars
    style.Colors[ImGuiCol_TitleBg]           = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]     = orange;
    style.Colors[ImGuiCol_TitleBgCollapsed]  = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);

    // Headers (used in menus, collapsing sections, etc.)
    style.Colors[ImGuiCol_Header]        = ImVec4(0.729f, 0.282f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = orange;
    style.Colors[ImGuiCol_HeaderActive]  = ImVec4(0.682f, 0.294f, 0.0f, 1.0f);

    // Frames (ComboBox, InputText, Sliders, ColorEdit, etc.)
    style.Colors[ImGuiCol_FrameBg]         = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered]  = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive]   = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

    // Sliders
    style.Colors[ImGuiCol_SliderGrab]        = orange;
    style.Colors[ImGuiCol_SliderGrabActive]  = ImVec4(0.9f, 0.45f, 0.1f, 1.0f);

    // Buttons
    style.Colors[ImGuiCol_Button]         = ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // Neutral gray
    style.Colors[ImGuiCol_ButtonHovered]  = orange;
    style.Colors[ImGuiCol_ButtonActive]   = ImVec4(0.682f, 0.294f, 0.0f, 1.0f);

    // Scrollbars
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = orange;
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(1.0f, 0.5f, 0.1f, 1.0f);

    // Text
    style.Colors[ImGuiCol_Text]          = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);  // Normal text
    style.Colors[ImGuiCol_TextDisabled]  = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);  // Disabled/grayed-out

    // Misc
    style.Colors[ImGuiCol_CheckMark] = orange;
    style.Colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
    style.Colors[ImGuiCol_Border]    = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

    // Resize
    style.Colors[ImGuiCol_ResizeGrip]         = ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // Default state
    style.Colors[ImGuiCol_ResizeGripHovered]  = orange;                           // Hovered
    style.Colors[ImGuiCol_ResizeGripActive]   = ImVec4(0.682f, 0.294f, 0.0f, 1.0f); // Active

    // Border
    style.Colors[ImGuiCol_Border]        = ImVec4(0.25f, 0.25f, 0.25f, 1.0f); // Normal edge
    style.Colors[ImGuiCol_BorderShadow]  = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);    // Drop shadow — set to 0 to disable
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

    PaError open_error = Pa_OpenStream(&mStream, &inputParams, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paNoFlag, AudioCallback, nullptr);
    if (open_error != paNoError) {
        SDL_Log("Failed to open stream: %s", Pa_GetErrorText(open_error));
    }
    PaError start_error = Pa_StartStream(mStream);
    if (start_error != paNoError) {
        SDL_Log("Failed to open stream: %s", Pa_GetErrorText(start_error));
    }
}

bool App::Initialize() {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }

    float display_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;

    mWindow = SDL_CreateWindow("Darktuna", 600, 300, window_flags);
    if (!mWindow) {
        SDL_Log("Failed to create SDL window: %s", SDL_GetError());
        return false;
    }

    // Set application icon
    SDL_Surface* icon = CreateSurfaceFromIcon();
    SDL_SetWindowIcon(mWindow, icon);
    SDL_DestroySurface(icon);

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
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    // Set up ImGui scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(display_scale);
    style.FontScaleDpi = display_scale;

    ApplyDarkboxTheme(style);
    
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

void App::ProcessEvent(SDL_Event *event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void App::BeginFrame() {
    SDL_Delay(10);
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

        if (mSignalStrength > mRmsThreshold) {
            float detectedFrequency = Tuner::DetectFrequencyAutocorrelation(mAudioBuffer, BUFFER_SIZE, SAMPLE_RATE);

            if (detectedFrequency > 20.0f && detectedFrequency < 500.0f) {
                mDetectedFrequency = detectedFrequency;
                mCurrentNote = &Tuner::GetClosestNote(mDetectedFrequency);
                mCentsOff = Tuner::GetCentsOff(mDetectedFrequency, mCurrentNote->freq);
            }
        }

        mIsReadyForProcessing = false;
    }

    if (mNumAudioDevices != Pa_GetDeviceCount()) {
        UpdateAudioDevices();
    }
}

void App::Draw() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if (mShowSettingsMenu) {
        if (ImGui::Begin("Settings")) {
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

            // Slider for RMS threshold
            ImGui::SliderFloat("RMS Threshold", &mRmsThreshold, 0.0f, 0.02f, "%.4f");

            // Slider for Cents Tolerance
            ImGui::SliderFloat("Cents Tolerance", &mCentsTolerance, 1.0f, 20.0f, "%.1f");

            ImGui::Separator();
            if (ImGui::Button("Close")) {
                mShowSettingsMenu = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset to defaults")) {
                mRmsThreshold = 0.01f;
                mCentsTolerance = 5.0f;
            }

            ImGui::End();
        }
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

        if (ImGui::BeginMenu("About")) {
            ImGui::MenuItem("Darktuna", nullptr, false, false);
            ImGui::MenuItem("by bottledlactose", nullptr, false, false);
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

    float content_width = 400.0f;
    float content_height = 120.0f;

    // Compute top-left corner for centered layout
    ImVec2 content_pos = ImVec2(
        (windowSize.x - content_width) * 0.5f,
        (windowSize.y - content_height) * 0.5f
    );

    // 4. Move the ImGui cursor to the center position
    ImGui::SetCursorPos(content_pos);
    ImGui::BeginChild("CenterContent", ImVec2(content_width, content_height), false, ImGuiWindowFlags_NoScrollbar);

    // Show RMS value
    ImGui::Text("Strength (RMS): %.6f\n", mSignalStrength);

    if (mCurrentNote) {
        ImGui::Text("Detected: %.2f Hz", mDetectedFrequency);
        ImGui::Text("Note: %s (%.2f Hz)", mCurrentNote->name.c_str(), mCurrentNote->freq);
        ImGui::Text("Cents off: %.2f", mCentsOff);

        // Color and tuning direction indicator
        if (std::abs(mCentsOff) < mCentsTolerance) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Green
            ImGui::Text("In tune");
            ImGui::PopStyleColor();
        } else if (mCentsOff > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 165, 0, 255)); // Orange
            ImGui::Text("Tune down (sharp)");
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 165, 0, 255)); // Orange
            ImGui::Text("Tune up (flat)");
            ImGui::PopStyleColor();
        }
    } else if (!mStream) {
        ImGui::Text("No active stream, please select an input device!");
    } else {
        ImGui::Text("Listening...");
    }

    if (!kGuitarTunings.empty()) {
        static int tuningIndex = 0;
        ImGui::Combo("Tuning", &tuningIndex, 
            [](void* data, int idx, const char** out_text) {
                *out_text = kGuitarTunings[idx].first.c_str();
                return true;
            }, nullptr, static_cast<int>(kGuitarTunings.size()));

        // Display note guide
        const auto& selectedTuning = kGuitarTunings[tuningIndex];
        ImGui::Text("Target Notes:");
        for (const auto& note : selectedTuning.second) {
            ImGui::SameLine();
            if (mCurrentNote && mCurrentNote->name == note) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 255, 255)); // Cyan
                ImGui::Text("%s", note.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::Text("%s", note.c_str());
            }
        }
    }

    ImGui::EndChild();
    ImGui::End();
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
    mNumAudioDevices = Pa_GetDeviceCount();
    mAudioDevices.clear();

    for (int i = 0; i < mNumAudioDevices; ++i) {
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
