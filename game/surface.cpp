#include "surface.h"

#include <cassert>

#include <functional>
#include <iostream>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include <SDL.h>

#include <game/color.h>
#include <game/rect.h>
#include <game/point.h>
#include <game/sdl_utils.h>

namespace
{
    struct null_pixelformat_error : public std::runtime_error
    {
         null_pixelformat_error() throw() : std::runtime_error("pixel format is null or invalid") {}
    };

    struct null_surface_error : public std::runtime_error
    {
         null_surface_error() throw() : std::runtime_error("surface is null or invalid") {}
    };

    struct null_pixeldata_error : public std::runtime_error
    {
         null_pixeldata_error() throw() : std::runtime_error("pixel pointer is null") {}
    };
        
    void AddSurfaceRef(SDL_Surface *surface)
    {
        if(surface != nullptr) {
            ++surface->refcount;
        }
    }
    
    class TransformFunctor
    {
        const SDL_PixelFormat *mFormat;
        std::function<Color(Color const&)> mFunc;

    public:
        TransformFunctor(const SDL_PixelFormat &format, Color func(Color const&))
            : mFormat(&format)
            , mFunc(func)
            { }

        void operator()(char *bytes, size_t size) {
            const char *end = bytes + size * mFormat->BytesPerPixel;
        
            while(bytes != end) {
                const uint32_t origPixel = GetPackedPixel(bytes, mFormat->BytesPerPixel);

                Color color;
                SDL_GetRGBA(origPixel, mFormat, &color.r, &color.g, &color.b, &color.a);

                const Color result = mFunc(color);
                const uint32_t pixel = SDL_MapRGBA(mFormat, result.r, result.g, result.b, result.a);
                SetPackedPixel(bytes, pixel, mFormat->BytesPerPixel);

                bytes += mFormat->BytesPerPixel;
            }
        }
    };

    class ConvolveFunctor
    {
        std::vector<uint32_t> mBuffer;
        uint32_t *const mRedBuff;
        uint32_t *const mGreenBuff;
        uint32_t *const mBlueBuff;
        const int mRadius;
        const SDL_PixelFormat *mFormat;

    public:
        ConvolveFunctor(int radius, const SDL_PixelFormat &format, int bufferSize)
            : mBuffer(bufferSize * 3)
            , mRedBuff(&mBuffer[bufferSize * 0])
            , mGreenBuff(&mBuffer[bufferSize * 1])
            , mBlueBuff(&mBuffer[bufferSize * 2])
            , mRadius(radius)
            , mFormat(&format)
            { }
        
        void operator()(char *bytes, int length, int stride)
        {    
            uint32_t redAccum = 0;
            uint32_t greenAccum = 0;
            uint32_t blueAccum = 0;

            for(int i = 0; i < length; ++i) {
                const uint32_t pixel = GetPackedPixel(bytes + i * stride, mFormat->BytesPerPixel);

                uint8_t r;
                uint8_t g;
                uint8_t b;
                SDL_GetRGB(pixel, mFormat, &r, &g, &b);

                redAccum += r;
                greenAccum += g;
                blueAccum += b;

                mRedBuff[i] = redAccum;
                mGreenBuff[i] = greenAccum;
                mBlueBuff[i] = blueAccum;
            }

            for(int i = 0; i < length; ++i) {
                const size_t minIndex = std::max(0, i - mRadius);
                const size_t maxIndex = std::min(i + mRadius, length - 1);

                const uint32_t red = mRedBuff[maxIndex] - mRedBuff[minIndex];
                const uint32_t green = mGreenBuff[maxIndex] - mGreenBuff[minIndex];
                const uint32_t blue = mBlueBuff[maxIndex] - mBlueBuff[minIndex];
                const uint32_t count = maxIndex - minIndex;

                const uint32_t pixel = SDL_MapRGB(mFormat, red / count, green / count, blue / count);
                SetPackedPixel(bytes + i * stride, pixel, mFormat->BytesPerPixel);
            }
        }
    };
}

SurfaceColorModSetter::SurfaceColorModSetter(const Surface &src, const Color &color)
{
    if(SDL_GetSurfaceColorMod(src, &redMod, &greenMod, &blueMod) < 0) {
        throw sdl_error();
    }
    if(SDL_SetSurfaceColorMod(src, color.r, color.g, color.b) < 0) {
        throw sdl_error();
    }
    surface = src;
}

SurfaceColorModSetter::~SurfaceColorModSetter()
{
    if(!surface.Null()) {
        SDL_SetSurfaceColorMod(surface, redMod, greenMod, blueMod);
    }
}

SurfaceLocker::SurfaceLocker(const Surface &surface)
    : mObject(surface)
    , mLocked(false)
{
    if(!mObject.Null()) {
        if(SDL_MUSTLOCK(mObject)) {
            if(0 == SDL_LockSurface(mObject))
                mLocked = true;
        }
    }
}

SurfaceLocker::~SurfaceLocker()
{
    if(mLocked && !mObject.Null()) {
        SDL_UnlockSurface(mObject);
    }
}

Surface::~Surface()
{
    SDL_FreeSurface(mSurface);
}

Surface::Surface()
    : mSurface(nullptr)
{
}

Surface::Surface(SDL_Surface *s)
    : mSurface(s)
{
}

Surface::Surface(const Surface &that)
    : mSurface(that.mSurface)
{
    AddSurfaceRef(mSurface);
}

Surface& Surface::operator=(SDL_Surface *s)
{
    Assign(s);
    return *this;
}

Surface& Surface::operator=(const Surface &that)
{
    AddSurfaceRef(that.mSurface);
    Assign(that.mSurface);
    return *this;
}

bool Surface::operator==(const Surface &that)
{
    return that.mSurface == mSurface;
}

void Surface::Assign(SDL_Surface *s)
{
    // SDL_FreeSurface manages refcount by itself
    // Suddenly.
    SDL_FreeSurface(mSurface);
    mSurface = s;
}

bool Surface::Null() const
{
    return (mSurface == nullptr);
}

SDL_Surface* Surface::operator->() const
{
    return mSurface;
}

Surface::operator SDL_Surface*() const
{
    return mSurface;
}

bool Surface::operator!() const
{
    return Null();
}

void Surface::Reset(SDL_Surface *surface)
{
    AddSurfaceRef(surface);
    Assign(surface);
}

Surface CreateSurfaceView(Surface &src, const Rect &clip)
{
    if(SDL_MUSTLOCK(src)) {
        // can we deal with it?
        throw std::invalid_argument("SurfaceView might not be created from RLEaccel surface");
    }

    const Rect cropped =
        IntersectRects(
            Normalized(clip),
            Rect(src));

    char *const pixels = GetPixels(src)
        + cropped.y * src->pitch
        + cropped.x * src->format->BytesPerPixel;

    Surface tmp = CreateSurfaceFrom(pixels, cropped.w, cropped.h, src->pitch, src->format);
    if(!tmp) {
        throw sdl_error();
    }
    CopyColorKey(src, tmp);
    
    return tmp;
}

SurfaceView::SurfaceView(Surface &src, const Rect &clip)
    : mSurface(CreateSurfaceView(src, clip))
    , mParentRef(src)
{
}

// \todo there is something wrong with const-specifier
SurfaceView::SurfaceView(const Surface &src, const Rect &clip)
    : SurfaceView(const_cast<Surface&>(src), clip)
{
}

void CopyColorKey(SDL_Surface *src, SDL_Surface *dst)
{
    uint32_t colorkey = 0;
    if(SDL_GetColorKey(src, &colorkey) == 0) {
        if(SDL_SetColorKey(dst, SDL_TRUE, colorkey) < 0) {
            throw sdl_error();
        }
    }
}

Surface CreateSurface(int width, int height, const SDL_PixelFormat *format)
{
    if(format != nullptr) {
        const uint32_t rmask = format->Rmask;
        const uint32_t gmask = format->Gmask;
        const uint32_t bmask = format->Bmask;
        const uint32_t amask = format->Amask;
        const int bpp = format->BitsPerPixel;
        return SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
    }
    
    throw null_pixelformat_error();
}

Surface CreateSurface(int width, int height, int format)
{
    uint32_t rmask;
    uint32_t gmask;
    uint32_t bmask;
    uint32_t amask;
    int bpp;

    if(!SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask)) {
        throw sdl_error();
    }
    
    return SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
}

Surface CreateSurfaceFrom(void *pixels, int width, int height, int pitch, const SDL_PixelFormat *format)
{
    if(format != nullptr) {
        const uint32_t rmask = format->Rmask;
        const uint32_t gmask = format->Gmask;
        const uint32_t bmask = format->Bmask;
        const uint32_t amask = format->Amask;
        const int bpp = format->BitsPerPixel;
        if(pixels != nullptr) {
            return SDL_CreateRGBSurfaceFrom(pixels, width, height, bpp, pitch, rmask, gmask, bmask, amask);
        }
        throw null_pixeldata_error();
    }
    throw null_pixelformat_error();
}

Surface CreateSurfaceFrom(void *pixels, int width, int height, int pitch, int format)
{
    if(pixels != nullptr) {
        uint32_t rmask;
        uint32_t gmask;
        uint32_t bmask;
        uint32_t amask;
        int bpp;

        if(!SDL_PixelFormatEnumToMasks(format, &bpp, &rmask, &gmask, &bmask, &amask)) {
            throw sdl_error();
        }
        
        return SDL_CreateRGBSurfaceFrom(pixels, width, height, bpp, pitch, rmask, gmask, bmask, amask);
    }
    throw null_pixeldata_error();
}

void BlitSurface(const Surface &src, const Rect &srcrect, Surface &dst, const Rect &dstrect)
{
    if(SDL_BlitSurface(src, &srcrect, dst, &const_cast<Rect&>(dstrect)) < 0) {
        throw sdl_error();
    }
}

void BlitSurfaceScaled(const Surface &src, const Rect &srcrect, Surface &dst, const Rect &dstrect)
{
    if(SDL_BlitScaled(src, &srcrect, dst, &const_cast<Rect&>(dstrect)) < 0) {
        throw sdl_error();
    }
}

void DrawFrame(Surface &dst, const Rect &dstrect, const Color &color)
{
    RendererPtr render(SDL_CreateSoftwareRenderer(dst));
    SDL_SetRenderDrawBlendMode(render.get(), SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(render.get(), color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(render.get(), &dstrect);
}

void FillFrame(Surface &dst, const Rect &dstrect, const Color &color)
{
    RendererPtr render(SDL_CreateSoftwareRenderer(dst));
    SDL_SetRenderDrawBlendMode(render.get(), SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(render.get(), color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(render.get(), &dstrect);
}

void DrawRhombus(Surface &dst, const Rect &bounds, const Color &color)
{
    if(!dst) {
        throw null_surface_error();
    }
    
    RendererPtr render(SDL_CreateSoftwareRenderer(dst));
    if(!render) {
        throw sdl_error();
    }
    
    SDL_SetRenderDrawBlendMode(render.get(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(render.get(), color.r, color.g, color.b, color.a);

    constexpr int numPoints = 5;
    const Point points[numPoints] = {
        Point(bounds.x, bounds.y + bounds.h / 2),
        Point(bounds.x + bounds.w / 2, bounds.y),
        Point(bounds.x + bounds.w, bounds.y + bounds.h / 2),
        Point(bounds.x + bounds.w / 2, bounds.y + bounds.h),
        Point(bounds.x, bounds.y + bounds.h / 2)
    };
    
    if(SDL_RenderDrawLines(render.get(), &points[0], numPoints) < 0) {
        throw sdl_error();
    }
}

void TransformSurface(Surface &dst, Color func(Color const&))
{
    if(!dst) {
        throw null_surface_error();
    }
    
    const SurfaceLocker lock(dst);
    char *bytes = GetPixels(dst);
    TransformFunctor transformBuffer(*dst->format, func);
    
    for(int i = 0; i < dst->h; ++i) {
        transformBuffer(bytes + i * dst->pitch, dst->w);
    }
}

void BlurSurface(Surface &dst, int radius)
{
    if(!dst) {
        throw null_surface_error();
    }
    
    const SurfaceLocker lock(dst);
    char *const bytes = GetPixels(dst);
    const int bytesPP = dst->format->BytesPerPixel;
    ConvolveFunctor convolve(radius, *dst->format, std::max(dst->w, dst->h));
    
    for(int y = 0; y < dst->h; ++y) {
        convolve(bytes + y * dst->pitch, dst->w, bytesPP);
    }

    for(int x = 0; x < dst->w; ++x) {
        convolve(bytes + x * bytesPP, dst->h, dst->pitch);
    }
}

uint32_t GetPixel(const Surface &surface, const Point &coord)
{
    if(!surface) {
        throw null_surface_error();
    }

    if(coord.x < 0 || coord.y < 0 || coord.x >= surface->w || coord.y >= surface->h) {
        throw std::invalid_argument("coord out of surface bounds");
    }
    
    const SurfaceLocker lock(surface);
    return GetPixelLocked(surface, coord);
}

uint32_t GetPixelLocked(const Surface &surface, const Point &coord)
{
    return GetPackedPixel(GetPixels(surface) + coord.y * surface->pitch + coord.x * surface->format->BytesPerPixel, surface->format->BytesPerPixel);
}

bool HasPalette(const Surface &surface)
{
    if(!surface) {
        throw null_surface_error();
    }
    return SDL_ISPIXELFORMAT_INDEXED(surface->format->format);
}
