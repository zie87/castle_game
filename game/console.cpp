#include "console.h"

#include <SDL.h>

#include <iostream>
#include <string>

#include <game/color.h>
#include <game/rect.h>
#include <game/fontmanager.h>
#include <game/screenmanager.h>
#include <game/surface.h>
#include <game/textrenderer.h>

namespace UI
{
    Console::Console(UI::ScreenManager &screenManager)
        : mScreenManager(screenManager)
        , mText("")
        , mFontName(Render::FontStronghold)
        , mFontSize(14)
        , mClosed(false)
    { }

    void Console::Draw(Surface &frame)
    {
        FillFrame(frame, Rect(frame->w, frame->h / 2), Colors::Black.Opaque(100));
        Render::TextRenderer textRenderer(frame);
        textRenderer.SetFont(Render::FindFont(mFontName, mFontSize));
        textRenderer.PutString(mText);
    }
    
    bool Console::HandleEvent(const SDL_Event &event)
    {
        switch(event.type) {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
            return HandleKey(event.key);
        case SDL_TEXTINPUT:
            return HandleTextInput(event.text);
        default:
            return false;
        }
    }

    bool Console::HandleKey(const SDL_KeyboardEvent &event)
    {
        switch(event.keysym.sym) {
        case SDLK_ESCAPE:
            mScreenManager.PopScreen();
            return true;
        case SDLK_RETURN:
            mText = std::string();
            return true;
        case SDLK_BACKSPACE:
            if(!mText.empty()) {
                mText.erase(mText.size() - 1);
            }
            return true;
        default:
            return false;
        }
    }

    bool Console::HandleTextInput(const SDL_TextInputEvent &text)
    {
        mText += text.text;
        return true;
    }
} // namespace UI