#pragma once

// Forward declarations
struct SDL_Window;
struct SDL_Renderer;

struct App {
private:
    SDL_Window *mWindow;
    SDL_Renderer *mRenderer;

    App() = default;
    App(const App&) = delete;
    App& operator=(const App&) = delete;

public:
    static App& Get() {
        static App instance;
        return instance;
    }

    bool Initialize();

    void BeginFrame();
    void EndFrame();

    inline SDL_Window *GetWindow() const {
        return mWindow;
    }

    inline SDL_Renderer *GetRenderer() const {
        return mRenderer;
    }
};
