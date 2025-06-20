#include "App.hpp"

#include "SDL3/SDL_init.h"
#include "SDL3/SDL_log.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"

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

    // TODO: Move PortAudio initialization here

    return true;
}
