#include "gamescreen.h"

#include <sstream>
#include <random>

#include <game/renderer.h>
#include <game/landscape.h>
#include <game/sdl_utils.h>
#include <game/filesystem.h>
#include <game/surface.h>
#include <game/collection.h>
#include <game/direction.h>

#include <game/gm1.h>
#include <game/gm1palette.h>

#include <game/fontmanager.h>
#include <game/textrenderer.h>

namespace UI
{
    GameScreen::GameScreen(UI::ScreenManager &screenMgr, Render::FontManager &fontMgr, Render::Renderer &render)
        : mScreenMgr(screenMgr)
        , mFontMgr(fontMgr)
        , mRenderer(render)
        , mCursorX(0)
        , mCursorY(0)
        , mCursorInvalid(true)
        , mViewportX(0)
        , mViewportY(0)
        , mViewportRadius(1)
        , mViewportOrient(Direction::North)
        , mFlatView(false)
        , mLowView(false)
        , mZoomedOut(false)
        , mHiddenUI(false)
        , mCursorMode(CursorMode::Normal)
        , mClosed(false)
        , mGameMap(400)
        , mSpriteCount(0)
    {
    }

    void GameScreen::DrawUnits(Surface &frame, const CollectionData &gm1)
    {
        std::default_random_engine g(0);
        std::uniform_int_distribution<int> paletteIndex(1, GM1::CollectionPaletteCount - 1);
        std::uniform_int_distribution<int> entryIndex(0, gm1.entries.size() - 1);

        const SDL_Rect frameRect = SurfaceBounds(frame);

        int x = 0;
        int y = 0;

        bool evenRow = true;

        for(;;) {
            SDL_Palette *palette = gm1.palettes.at(paletteIndex(g)).get();
            const CollectionEntry &entry = gm1.entries.at(entryIndex(g));
        
            Surface face = entry.surface;
            SDL_SetSurfacePalette(face, palette);
            
            x += GM1::TileWidth;
            if(x > frameRect.w + face->w) {
                evenRow = !evenRow;
                y += GM1::TileHeight / 2;
                x = (evenRow ? (GM1::TileWidth / 2) : 0);
            }

            if(y > frameRect.h) {
                break;
            }
            
            SDL_Rect whither = MakeRect(x, y, face->w, face->h);
            BlitSurface(face, NULL, frame, &whither);
            mSpriteCount++;
        }
    }

    void GameScreen::DrawTerrain(Surface &frame, const CollectionData &gm1)
    {
        int x = 0;
        int y = 0;

        bool evenRow = true;
        
        const SDL_Rect frameRect = SurfaceBounds(frame);

        std::default_random_engine g(0);
        std::uniform_int_distribution<int> entryIndex(0, gm1.entries.size() - 1);
        
        for(;;) {
            const CollectionEntry &entry = gm1.entries.at(entryIndex(g));
            Surface face = entry.surface;
            
            x += GM1::TileWidth;
            if(x > frameRect.w + face->w) {
                evenRow = !evenRow;
                y += GM1::TileHeight / 2;
                x = (evenRow ? (GM1::TileWidth / 2) : 0);
            }

            if(y > frameRect.h) {
                break;
            }

            SDL_Rect whither = MakeRect(x - gm1.header.anchorX, y - gm1.header.anchorY - entry.header.tileY, face->w, face->h);
            BlitSurface(face, NULL, frame, &whither);
            mSpriteCount++;
        }
    }
    
    void GameScreen::DrawTestScene(Surface &frame)
    {
        mSpriteCount = 0;
        
        static fs::path tileLand = fs::GM1FilePath("tile_land8");
        static const CollectionData &tileLandGM1 = mRenderer.QueryCollection(tileLand);
        DrawTerrain(frame, tileLandGM1);
        
        static fs::path bodyLord = fs::GM1FilePath("body_archer");
        static const CollectionData &bodyLordGM1 = mRenderer.QueryCollection(bodyLord);
        DrawUnits(frame, bodyLordGM1);

        Render::TextRenderer textRenderer(frame);
        textRenderer.Translate(0, frame->h - 20);
        textRenderer.SetColor(MakeColor(255, 0, 0, 255));
        textRenderer.SetFont(mFontMgr.Font(Render::FontStronghold, 10));

        std::ostringstream oss;
        oss << "Scene objects: " << mSpriteCount;
        FillFrame(frame, textRenderer.CalculateTextRect(oss.str()), MakeColor(0, 0, 0, 100));
        textRenderer.PutString(oss.str());
    }

    void GameScreen::Draw(Surface &frame)
    {
        DrawTestScene(frame);
        return;
        mGameMap.Draw(frame, mViewportX, mViewportY, mViewportOrient, mViewportRadius);
    }
    
    bool GameScreen::HandleEvent(const SDL_Event &event)
    {
        switch(event.type) {
        case SDL_MOUSEMOTION:
            {
                mCursorInvalid = false;
                mCursorX = event.motion.x;
                mCursorY = event.motion.y;
            }
            break;
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    return false;
                default:
                    return true;
                }
            }
            break;
        }
        return false;
    }

    void GameScreen::AdjustViewport(const SDL_Rect &screen)
    {
        if(screen.w == mCursorX)
            ++mViewportX;
        else if(0 == mCursorX)
            --mViewportX;

        if(screen.h == mCursorY)
            ++mViewportY;
        else if(0 == mCursorY)
            --mViewportY;
    }

} // namespace UI
