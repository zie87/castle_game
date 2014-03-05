#ifndef SDL_INIT_H_
#define SDL_INIT_H_

#include <stdexcept>
#include <SDL2/SDL.h>

class SDLInit
{
public:
    SDLInit(int flags) throw(std::runtime_error);
    ~SDLInit() throw();
};

#endif