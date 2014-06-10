#include "engine.h"

#include <iomanip>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include <SDL.h>

#include <game/fontmanager.h>
#include <game/make_unique.h>
#include <game/color.h>
#include <game/rect.h>
#include <game/point.h>
#include <game/collection.h>
#include <game/surface_drawing.h>
#include <game/gamescreen.h>
#include <game/renderer.h>
#include <game/screen.h>
#include <game/gamemap.h>

namespace Castle
{
    Engine::Engine()
        : mSDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE)
        , mRenderer()
        , mFpsAverage(0.0f)
        , mFrameCounter(0)
        , mClosed(false)
        , mFrameUpdateInterval(std::chrono::milliseconds(0))
        , mFpsUpdateInterval(std::chrono::seconds(3))
        , mFpsLimited(false)
        , mIO()
        , mPort(4500)
        , mScreenManager()
        , mServer(mIO, mPort)
        , mGraphicsMgr()
        , mInfoArea()
    {
        mInfoArea.SetTextColor(Colors::Red);
        mInfoArea.SetBackgroundColor(Colors::Black.Opaque(160));
        mInfoArea.SetText("No FPS for you, Sir");
        mInfoArea.SetMaxWidth(200);
    }

    bool Engine::HandleWindowEvent(const SDL_WindowEvent &window)
    {
        switch(window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            {
                const int width = window.data1;
                const int height = window.data2;
                ResizeScreen(width, height);
            }
            return true;
        default:
            return true;
        }
    }

    bool Engine::HandleKeyboardEvent(const SDL_KeyboardEvent &key)
    {
        switch(key.keysym.sym) {
        case SDLK_ESCAPE:
            mClosed = true;
            return false;
        case SDLK_TAB:
            mScreenManager.ToggleConsole();
            return true;
        default:
            return true;
        }
    }

    bool Engine::HandleEvent(const SDL_Event &event)
    {
        switch(event.type) {
        case SDL_WINDOWEVENT:
            return HandleWindowEvent(event.window);
        case SDL_QUIT:
            mClosed = true;
            return false;
        case SDL_KEYDOWN:
            return HandleKeyboardEvent(event.key);
        default:
            return true;
        }
    }

    void Engine::ResizeScreen(int width, int height)
    {
        mRenderer.SetScreenSize(width, height);
    }
    
    void Engine::LoadFonts()
    {
        const int minHeight = 8;
        const int maxHeight = 42;
        
        const std::string family = Render::RegularFont;

        for(int h = minHeight; h <= maxHeight; ++h) {
            try {
                mRenderer.GetFontManager().LoadFont(UI::Font(family, h));
            } catch(const std::exception &error) {
                std::cerr << "Load font failed: " << error.what() << std::endl;
            }
        }
    }

    void Engine::LoadGraphics()
    {
        // precache gm dir
    }
    
    void Engine::PollInput()
    {
        try {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(!mScreenManager.TopScreen().HandleEvent(event)) {
                    HandleEvent(event);
                }
            }
        } catch(const std::exception &error) {
            std::cerr << "exception on input handling: " << error.what() << std::endl;
        } catch(...) {
            std::cerr << "unknown exception on input handling" << std::endl;
        }
    }
    
    void Engine::PollNetwork()
    {
        try {
            mIO.poll();
        } catch(const std::exception &error) {
            std::cerr << "exception on polling network: " << error.what() << std::endl;
        } catch(...) {
            std::cerr << "unknown exception on polling network" << std::endl;
        }
        
        // forward connections to simulation manager
        // forward data to simulation manager
    }
    
    void Engine::DrawFrame()
    {
        mRenderer.BeginFrame();
        mScreenManager.Render(mRenderer);
        if(&mScreenManager.TopScreen() != &mScreenManager.Console()) {
            mInfoArea.Render(mRenderer);
        }
        mRenderer.DrawFrame(mRenderer.GetScreenRect(), Colors::Gray);
        mRenderer.EndFrame();
    }

    void Engine::UpdateFrameCounter(std::chrono::milliseconds elapsed)
    {
        const std::chrono::milliseconds oneSecond = std::chrono::seconds(1);
        const double preciseFrameCounter = mFrameCounter * oneSecond.count();
        mFpsAverage = preciseFrameCounter / elapsed.count();
        mFrameCounter = 0;

        std::ostringstream oss;
        oss << "Your FPS, sir: " << std::setw(10) << mFpsAverage;
        mInfoArea.SetText(oss.str());
    }

    constexpr std::chrono::milliseconds Elapsed(const std::chrono::steady_clock::time_point &lhs,
                                                const std::chrono::steady_clock::time_point &rhs)
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(rhs - lhs);
    }
    
    int Engine::Exec()
    {
        using namespace std::chrono;
        
        LoadFonts();
        LoadGraphics();
        SimulationManager::Instance().SetGameMap(std::make_unique<GameMap>(100));
        GenerateRandomMap(SimulationManager::Instance().GetGameMap());
        mScreenManager.EnterGameScreen();
        mServer.StartAccept();
        
        steady_clock::time_point prevSimulation = steady_clock::now();
        steady_clock::time_point prevFrame = steady_clock::now();
        steady_clock::time_point prevFpsUpdate = steady_clock::now();
        
        while(!mClosed) {
            PollInput();
            PollNetwork();

            {
                const steady_clock::time_point now = steady_clock::now();
                if(!mFpsLimited || prevFrame + mFrameUpdateInterval < now) {
                    mFrameCounter += 1;
                    prevFrame = now;
                    // try {
                    DrawFrame();
                    // } catch(const std::exception &error) {
                    //     std::cerr << "DrawFrame failed: " << error.what() << std::endl;
                    // }
                }
            }

            {
                const steady_clock::time_point now = steady_clock::now();
                milliseconds sinceLastSim = Elapsed(prevSimulation, now);
                if(SimulationManager::Instance().HasUpdate(sinceLastSim)) {
                    prevSimulation = now;
                    SimulationManager::Instance().Update();
                }
            }

            {
                const steady_clock::time_point now = steady_clock::now();
                const milliseconds sinceLastFpsUpdate = Elapsed(prevFpsUpdate, now);
                if(mFpsUpdateInterval < sinceLastFpsUpdate) {
                    prevFpsUpdate = now;
                    UpdateFrameCounter(sinceLastFpsUpdate);
                }
            }
        }

        return 0;
    }
}
