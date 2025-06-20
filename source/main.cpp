#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

#include "App.hpp"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    App::Get().Initialize();
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    App::Get().BeginFrame();
    App::Get().Update();
    App::Get().Draw();
    App::Get().EndFrame();
    return SDL_APP_CONTINUE;    
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    
    switch (event->type) {
        case SDL_EVENT_QUIT:
            return SDL_APP_SUCCESS;
    }

    App::Get().ProcessEvent(event);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    App::Get().Shutdown();
}
