#ifndef LOADINGSCREEN_H_
#define LOADINGSCREEN_H_

#include <memory>
#include <set>

#include <SDL2/SDL.h>

#include "events.h"
#include "gamescreen.h"
#include "game.h"
#include "errors.h"
#include "screen.h"
#include "gm1.h"
#include "renderer.h"
#include "textrenderer.h"
#include "font.h"

int RunLoadingScreen(Renderer &renderer);

#endif
