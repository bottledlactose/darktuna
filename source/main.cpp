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

#include "App.hpp"



SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    App::Get().Initialize();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {

    SDL_Delay(10);

    App::Get().BeginFrame();
    App::Get().Update();
    App::Get().Draw();
    App::Get().EndFrame();

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
    App::Get().Shutdown();
}
