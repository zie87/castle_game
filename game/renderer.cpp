#include "renderer.h"

#include <string>

#include <boost/current_function.hpp>
#include <boost/algorithm/clamp.hpp>

#include <game/sdl_error.h>
#include <game/sdl_utils.h>
#include <game/collection.h>

namespace
{
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
        , mScreenFormat(SDL_PIXELFORMAT_ARGB8888)
        , mScreenTexture(nullptr)
        , mScreenSurface(nullptr)
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

        mScreenFormat = format;
        mScreenWidth = width;
        mScreenHeight = height;
        mScreenTexture = std::move(temp);
    }

    void Renderer::CreateScreenSurface(int width, int height)
    {
        Surface temp = CreateSurface(width, height, mScreenFormat);
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

    Rect Renderer::GetScreenSize() const
    {
        Rect size;
        
        if(SDL_GetRendererOutputSize(mRenderer.get(), &size.w, &size.h) < 0) {
            throw sdl_error();
        }
        
        return size;
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

    void Renderer::PaintImage(const Surface &surface, const Rect &whither)
    {
        BlitSurface(surface, Rect(surface), mScreenSurface, whither);
    }
} // namespace Render
