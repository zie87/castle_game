#include "renderer.h"

#include <memory>
#include <string>

#include <cassert>

#include <boost/algorithm/clamp.hpp>

#include <game/make_unique.h>

#include <game/rect.h>
#include <game/point.h>
#include <game/color.h>
#include <game/gm1palette.h>
#include <game/sdl_error.h>
#include <game/sdl_utils.h>
#include <game/collection.h>
#include <game/surface_drawing.h>

namespace
{
    const uint32_t ScreenPixelFormat = SDL_PIXELFORMAT_RGB888;
    const int WindowWidth = 1024;
    const int WindowHeight = 768;

    const int WindowXPos = SDL_WINDOWPOS_UNDEFINED;
    const int WindowYPos = SDL_WINDOWPOS_UNDEFINED;

    const char *WindowTitle = "Castle game";

    const int WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

    const int RendererIndex = -1;
    const int RendererFlags = SDL_RENDERER_ACCELERATED;
    
    const int MinScreenWidth = 0;
    const int MinScreenHeight = 0;
    
    // TODO sync with driver's texture max width and height
    const int MaxScreenWidth = 4096;
    const int MaxScreenHeight = 4096;

    int AdjustWidth(int width)
    {
        return boost::algorithm::clamp(width, MinScreenWidth, MaxScreenWidth);
    }

    int AdjustHeight(int height)
    {
        return boost::algorithm::clamp(height, MinScreenHeight, MaxScreenHeight);
    }
}

namespace Render
{    
    Renderer::Renderer()
        : mScreenWidth(WindowWidth)
        , mScreenHeight(WindowHeight)
        , mScreenFormat(ScreenPixelFormat)
        , mScreenTexture(nullptr)
        , mScreenSurface(nullptr)
        , mEmptyPalette()
        , mBoundPalette(mEmptyPalette)
        , mBoundTexture()
        , mBoundAlphaMap()
        , mDefaultAlphaMod(255)
        , mAlphaMod(mDefaultAlphaMod)
        , mDefaultColorMod(255, 255, 255, 255)
        , mColorMod(mDefaultColorMod)
    {
        mWindow.reset(
            SDL_CreateWindow(WindowTitle,
                             WindowXPos,
                             WindowYPos,
                             mScreenWidth,
                             mScreenHeight,
                             WindowFlags));
        
        if(!mWindow) {
            throw sdl_error();
        }
        
        mRenderer.reset(
            SDL_CreateRenderer(mWindow.get(),
                               RendererIndex,
                               RendererFlags));
        if(!mRenderer) {
            throw sdl_error();
        }
    }

    void Renderer::CreateScreenTexture(int width, int height, int format)
    {
        if((mScreenTexture) && (width == mScreenWidth) && (height == mScreenHeight) && (format == mScreenFormat)) {
            return;
        }

        TexturePtr temp(
            SDL_CreateTexture(
                mRenderer.get(),
                format,
                SDL_TEXTUREACCESS_STREAMING,
                width,
                height));

        // Let sdl check the size and the format for us
        if(!temp) {
            throw sdl_error();
        }

        mScreenTexture = std::move(temp);
        mScreenFormat = format;
        mScreenWidth = width;
        mScreenHeight = height;
    }

    void Renderer::CreateScreenSurface(int width, int height)
    {
        const Surface temp = CreateSurface(width, height, mScreenFormat);
        if(!temp) {
            throw sdl_error();
        }
        
        mScreenSurface = temp;
    }

    bool Renderer::ReallocationRequired(int width, int height, int format)
    {
        return (width != mScreenWidth) || (height != mScreenHeight) || (format != mScreenHeight);
    }
    
    const Surface Renderer::BeginFrame()
    {
        if(!mScreenTexture) {
            CreateScreenTexture(mScreenWidth, mScreenHeight, mScreenFormat);
        }

        if(!mScreenSurface) {
            CreateScreenSurface(mScreenWidth, mScreenHeight);
        }
        
        SDL_FillRect(mScreenSurface, NULL, 0);
        
        return mScreenSurface;
    }

    void Renderer::EndFrame()
    {
        if(!mScreenSurface.Null()) {
            if(SDL_UpdateTexture(mScreenTexture.get(), NULL, mScreenSurface->pixels, mScreenSurface->pitch) < 0) {
                throw sdl_error();
            }

            const Rect textureRect(mScreenSurface);
            if(SDL_RenderCopy(mRenderer.get(), mScreenTexture.get(), &textureRect, &textureRect) < 0) {
                throw sdl_error();
            }
        }

        SDL_RenderPresent(mRenderer.get());
    }

    const Point Renderer::GetOutputSize() const
    {
        Point size;
        
        if(SDL_GetRendererOutputSize(mRenderer.get(), &size.x, &size.y) < 0) {
            throw sdl_error();
        }
        
        return size;
    }
    
    const Rect Renderer::GetScreenRect() const
    {
        return Rect(mScreenSurface);
    }

    void Renderer::SetScreenMode(int width, int height, int format)
    {
        if(ReallocationRequired(width, height, format)) {
            CreateScreenTexture(width, height, format);
            CreateScreenSurface(width, height);
        }
    }

    void Renderer::SetScreenFormat(int format)
    {
        SetScreenMode(mScreenWidth, mScreenHeight, format);
    }
    
    void Renderer::SetScreenSize(int width, int height)
    {
        // Cutting up and down texture height and width
        const int newWidth = AdjustWidth(width);
        const int newHeight = AdjustHeight(height);

        SetScreenMode(newWidth, newHeight, mScreenFormat);
    }
    
    const Surface Renderer::CreateImage(int width, int height, int format)
    {
        return CreateSurface(width, height, format);
    }

    const Surface Renderer::CreateImageFrom(int width, int height, int pitch, int format, char *data)
    {
        return CreateSurfaceFrom(data, width, height, pitch, format);
    }

    void Renderer::BindTexture(const Surface &surface)
    {
        mBoundTexture = surface;
    }
    
    void Renderer::BindPalette(const GM1::Palette &palette)
    {
        mBoundPalette = palette;
    }
    
    void Renderer::BindAlphaMap(const Surface &surface)
    {
        mBoundAlphaMap = surface;
    }

    void Renderer::UnbindTexture()
    {
        mBoundTexture = nullptr;
    }

    void Renderer::UnbindPalette()
    {
        mBoundPalette = mEmptyPalette;
    }

    void Renderer::UnbindAlphaMap()
    {
        mBoundAlphaMap = nullptr;
    }

    void Renderer::SetAlphaMod(int alpha)
    {
        mAlphaMod = alpha;
    }
    
    void Renderer::UnsetAlphaMod()
    {
        mAlphaMod = mDefaultAlphaMod;
    }

    void Renderer::SetColorMod(const Color &colorMod)
    {
        mColorMod = colorMod;
    }

    void Renderer::UnsetColorMod()
    {
        mColorMod = mDefaultColorMod;
    }
    
    void Renderer::BlitTexture(const Rect &textureSubRect, const Rect &screenSubRect)
    {
        SDL_SetSurfaceColorMod(mBoundTexture, mColorMod.r, mColorMod.g, mColorMod.b);

        if(HasPalette(mBoundTexture) && IsRGB(*mScreenSurface->format)) {
            SDL_SetSurfacePalette(mBoundTexture, &mBoundPalette.asSDLPalette());
        }
        
        BlitSurface(mBoundTexture, textureSubRect, mScreenSurface, screenSubRect);
    }

    void Renderer::FillRhombus(const Rect &bounds, const Color &bg)
    {
        Graphics::FillRhombus(mScreenSurface, bounds, bg);
    }
    
    void Renderer::DrawRhombus(const Rect &bounds, const Color &fg)
    {
        Graphics::DrawRhombus(mScreenSurface, bounds, fg);
    }

    void Renderer::FillFrame(const Rect &bounds, const Color &bg)
    {
        Graphics::FillFrame(mScreenSurface, bounds, bg);
    }

    void Renderer::DrawFrame(const Rect &bounds, const Color &fg)
    {
        Graphics::DrawRhombus(mScreenSurface, bounds, fg);
    }
} // namespace Render
