#include "main.h"

#include <cstring>

#include <sstream>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <SDL.h>

#include <game/sdl_utils.h>
#include <game/image.h>
#include <game/tgx.h>

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout << "Use string, luke!" << std::endl;
    }
    
    std::string name = argv[1];

    std::ifstream fin(name, std::ios_base::binary);
    if(!fin.is_open()) {
        std::ostringstream oss;
        oss << "can't open file: " << strerror(errno);
        throw std::runtime_error(oss.str());
    }
    
    castle::Image surf;
    TGX::ReadImageHeader(fin, surf);

    std::streampos origin = fin.tellg();
    fin.seekg(0, std::ios_base::end);
    std::streampos fsize = fin.tellg();
    fin.seekg(origin);
    
    TGX::DecodeImage(fin, fsize, surf);
    
    return ShowImage(surf);
}

int ShowImage(const castle::Image &surface)
{
    SDLInitializer init();

    SDL_Window *window = NULL;
    SDL_renderer *renderer = NULL;

    SDL_CreateWindowAndrenderer(surface->w, surface->h, 0, &window, &renderer);
    
    if(!window || !renderer) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    bool quit = false;
    while(!quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QuiT)
                quit = true;
        }
        SDL_renderopy(renderer, texture, NULL, NULL);
        SDL_renderresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_Destroyrenderer(renderer);
    SDL_DestroyWindow(window);
    
    return 0;
}
