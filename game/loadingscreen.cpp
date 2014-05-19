#include "loadingscreen.h"

#include <memory>
#include <sstream>
#include <iostream>

#include <SDL.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/clamp.hpp>

#include <game/color.h>
#include <game/rect.h>
#include <game/filesystem.h>
#include <game/collection.h>

namespace UI
{
    LoadingScreen::LoadingScreen()
        : mBackground(
            LoadTGX(
                fs::TGXFilePath("frontend_loading")))
        , mProgressDone(0)
        , mProgressMax(1)
    {
    }

    bool LoadingScreen::HandleEvent(SDL_Event const&)
    {
        return false;
    }

    void LoadingScreen::IncreaseDone(int delta)
    {
        mProgressDone += delta;
    }
    
    void LoadingScreen::SetProgressDone(int done)
    {
        mProgressDone = done;
    }

    void LoadingScreen::SetProgressMax(int max)
    {
        mProgressMax = max;
    }

    void LoadingScreen::SetProgressLabel(const std::string &text)
    {
        mStage = text;
    }
    
    double LoadingScreen::GetCompleteRate() const
    {
        double done = static_cast<double>(mProgressDone) / mProgressMax;
        return boost::algorithm::clamp(done, 0.0f, 1.0f);
    }
    
    void LoadingScreen::Draw(Surface &frame)
    {
        double rate = GetCompleteRate();
        
        Rect frameRect(frame);
        Rect bgRect(mBackground);

        Rect bgAligned = PutIn(bgRect, frameRect, 0, 0);
        BlitSurface(mBackground, bgRect, frame, bgAligned);
    
        Rect barOuter(300, 25);
        Rect barOuterAligned = PutIn(barOuter, bgAligned, 0, 0.8f);
        FillFrame(frame, barOuterAligned, Color::Black().Opaque(100));
        DrawFrame(frame, barOuterAligned, Color::Black());

        Rect barOuterPadded = PadIn(barOuterAligned, 5);
        Rect barInner(barOuterPadded.w * rate, barOuterPadded.h);
        Rect barInnerAligned = PutIn(barInner, barOuterPadded, -1.0f, 0);
        FillFrame(frame, barInnerAligned, Color::Black());
    }   
} // namespace UI
