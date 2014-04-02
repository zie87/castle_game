#ifndef GAMEMAP_H_
#define GAMEMAP_H_

#include <vector>
#include <SDL.h>
#include "landscape.h"

struct GameMapLocation
{
    GameMapLocation()
        : x(-1), y(-1) {}
    int x;
    int y;
    bool Null() const {
        return (x < 0) || (y < 0);
    }
};

class GameMap
{
    std::vector<int> mHeightMap;
    std::vector<int> mShadowMap;
    std::vector<LandscapeClass> mLandscape;

public:
    GameMap();
    ~GameMap();
    SDL_Rect GetLocationRect(const GameMapLocation &loc) const;
    GameMapLocation GetLocationAtPoint(int x, int y) const;
    
};

#endif