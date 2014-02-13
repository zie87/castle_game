#include "gamescreen.h"

static const int WORK = 1000;

void GameScreen::Draw(SDLRenderer &renderer)
{
    auto name = std::string("gm/anim_castle.gm1");

    renderer.BeginFrame();
    for(size_t i = 0; i < WORK; ++i) {
        auto rect = renderer.QuerySurfaceRect(name, rand() % 100);

        SDL_Rect src = MakeRect(
            rand() % 1200, rand() % 1000,
            rand() % 100, rand() % 100);
        
        renderer.CopyDrawingPlain(name, &rect, &src);
    }
    renderer.EndFrame();
}

void GameScreen::OnMouseMotion(const SDL_MouseMotionEvent &event)
{

}

void GameScreen::OnMouseButtonDown(const SDL_MouseButtonEvent &event)
{
    
}

void GameScreen::OnMouseButtonUp(const SDL_MouseButtonEvent &event)
{

}

void GameScreen::OnKeyDown(const SDL_KeyboardEvent &event)
{
    SDL_Log("Key down: %c", (char)event.keysym.sym);
}

void GameScreen::OnKeyUp(const SDL_KeyboardEvent &event)
{

}

std::unique_ptr<Screen> GameScreen::NextScreen()
{
    return nullptr;
}