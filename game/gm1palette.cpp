#include "gm1palette.h"

#include <SDL.h>

#include <iostream>
#include <algorithm>
#include <array>
#include <vector>

#include <game/modulo.h>
#include <game/tgx.h>
#include <game/sdl_utils.h>

namespace GM1
{
    PixelFormatPtr PaletteFormat()
    {
        PixelFormatPtr ptr(SDL_AllocFormat(TGX::PixelFormatEnum));
        if(!ptr) {
            throw sdl_error();
        }
        return ptr;
    }

    std::ostream& PrintPalette(std::ostream &out, const Palette &palette)
    {
        int column = 0;
        out << std::hex;
        for(palette_entry_t entry : palette) {
            out << entry << ' ';
            ++column;
            if(core::Mod(column, 16) == 0)
                out << std::endl;
        }
        return out;
    }
    
    PalettePtr CreateSDLPalette(const GM1::Palette &palette)
    {
        PixelFormatPtr &&format = GM1::PaletteFormat();
        std::vector<SDL_Color> colors(palette.size());
        std::transform(palette.begin(), palette.end(), colors.begin(),
                       [&format](palette_entry_t entry) -> SDL_Color {
                           SDL_Color result;
                           SDL_GetRGBA(entry, format.get(), &result.r, &result.g, &result.b, &result.a);
                           return result;
                       });
        return CreateSDLPalette(colors);
    }

    PalettePtr CreateSDLPalette(const std::vector<SDL_Color> &colors)
    {
        PalettePtr ptr(SDL_AllocPalette(colors.size()));
        if(!ptr) {
            throw sdl_error();
        }
        if(!SDL_SetPaletteColors(ptr.get(), colors.data(), 0, ptr->ncolors) < 0) {
            throw sdl_error();
        }
        return ptr;
    }
    
}
