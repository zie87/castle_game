#include "sdl_utils.h"
#include <iostream>
#include <initializer_list>

void ThrowSDLError(const SDL_Surface *surface)
{
    if(surface == NULL) {
        throw std::runtime_error(SDL_GetError());
    }
}

void ThrowSDLError(int code)
{
    if(code < 0) {
        throw std::runtime_error(SDL_GetError());
    }
}

SDL_Color MakeColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    return SDL_Color { r, g, b, a };
}

void PrintRendererInfo(std::ostream &out, const SDL_RendererInfo &info)
{
    using namespace std;
    
    out << "\tname: "
        << info.name
        << endl;
            
    out << "\tnumTexture_formats: "
        << dec
        << info.num_texture_formats
        << endl;
            
    out << "\tmax_texture_width: "
        << dec
        << info.max_texture_width
        << endl;
            
    out << "\tmax_texture_height: "
        << dec
        << info.max_texture_height
        << endl;

    out << "\ttexture_formats: ";
    if(info.num_texture_formats == 0) {
        out << "None";
    } else {
        for(size_t index = 0; index < info.num_texture_formats; ++index) {
            out << hex
                << info.texture_formats[index]
                << " ";
        }
    }
    out << endl;

    out << "\tflags: ";
    if(info.flags != 0) {
        if(info.flags & SDL_RENDERER_SOFTWARE)
            out << "SDL_RENDERER_SOFTWARE" << " | ";
        if(info.flags & SDL_RENDERER_ACCELERATED)
            out << "SDL_RENDERER_ACCELERATED" << " | ";
        if(info.flags & SDL_RENDERER_PRESENTVSYNC)
            out << "SDL_RENDERER_PRESENTVSYNC" << " | ";
        if(info.flags & SDL_RENDERER_TARGETTEXTURE)
            out << "SDL_RENDERER_TARGETTEXTURE" << " | ";
    } else {
        out << "None";
    }
    out << endl;
}

std::ostream &operator<<(std::ostream &out, const SDL_Rect &rect)
{
    out << rect.w << 'x' << rect.h;
    if(rect.x >= 0)
        out << '+';
    out << rect.x;
    if(rect.y >= 0)
        out << '+';
    out << rect.y;
    return out;
}

std::ostream &operator<<(std::ostream &out, const SDL_Point &pt)
{
    out << '(' << pt.x << ", " << pt.y << ')';
    return out;
}
