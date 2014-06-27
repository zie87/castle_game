#include "imageview.h"

#include <SDL.h>

#include <stdexcept>

#include <game/imagelocker.h>
#include <core/color.h>
#include <game/sdl_error.h>
#include <core/rect.h>

namespace castle
{
    const Image CreateImageView(Image &src, const core::Rect &clip)
    {
        if(SDL_MUSTLOCK(src.GetSurface())) {
            // \todo can we deal with it?
            throw std::invalid_argument("ImageView might not be created from RLEaccel surface");
        }

        ImageLocker lock(src);
        
        const core::Rect cropped =
            core::Intersection(
                core::Normalized(clip),
                core::Rect(src.Width(), src.Height()));

        char *const data = lock.Data()
            + cropped.Y() * src.RowStride()
            + cropped.X() * src.PixelStride();

        Image tmp = CreateImageFrom(data, cropped.Width(), cropped.Height(), src.RowStride(), ImageFormat(src));
        if(src.ColorKeyEnabled()) {
            tmp.SetColorKey(src.GetColorKey());
        }
    
        return tmp;
    }

    ImageView::ImageView(Image &src, const core::Rect &clip)
        : mImage(CreateImageView(src, clip))
        , mParentRef(src)
    {
    }

// \todo there is something wrong with const-specifier
// problem arises to SDL_CreateRGBSurfaceFrom which asks for non-const pixels
// We can just copy object to remove const-cv anyway.
    ImageView::ImageView(const Image &src, const core::Rect &clip)
        : ImageView(const_cast<Image&>(src), clip)
    {
    }
}
