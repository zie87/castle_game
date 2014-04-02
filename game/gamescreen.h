#ifndef GAMESCREEN_H_
#define GAMESCREEN_H_

#include <memory>
#include <SDL.h>
#include "screen.h"
#include "gamemap.h"

enum class Direction;
class Surface;

enum class CursorMode : int {
    Normal,
    Delete
};

namespace Castle
{
    class Engine;
}

namespace Render
{
    class Renderer;
}

namespace UI
{

    class ScreenManager;
    
    class GameScreen : public Screen
    {
        UI::ScreenManager *mScreenMgr;
        Render::Renderer *mRenderer;
        GameMap mMap;
        int mCursorX;
        int mCursorY;
        bool mCursorInvalid;
        int mViewportX;
        int mViewportY;
        int mViewportRadius;
        Direction mViewportOrient;
        bool mFlatView;
        bool mLowView;
        bool mZoomedOut;
        bool mHiddenUI;
        CursorMode mCursorMode;
        bool mClosed;
    
    public:
        GameScreen(UI::ScreenManager *mgr, Render::Renderer *render);
        GameScreen(const GameScreen &that) = delete;
        GameScreen &operator=(const GameScreen &that) = delete;
        ~GameScreen();
    
        void Draw(Surface &frame);
        bool HandleEvent(const SDL_Event &event);

        void AdjustViewport(const SDL_Rect &screen);
    
    };
    
}

#endif