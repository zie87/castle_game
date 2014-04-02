#ifndef GRAPHICSMANAGER_H_
#define GRAPHICSMANAGER_H_

#include <unordered_map>
#include <string>
#include <utility>
#include <cstddef>
#include <set>

#include "make_unique.h"
#include "gm1reader.h"
#include "gm1.h"
#include "filesystem.h"
#include "surface.h"

namespace UI
{
    class LoadingScreen;
}

namespace Render
{
    class Renderer;
}

namespace Render
{
    using GraphicsID = size_t;
    
    class GraphicsManager
    {
        Renderer *mRenderer;
        size_t mPaletteIndex;
        size_t mUniqueGraphicsID;
        std::unordered_map<std::string, GM1::GM1Reader> mReaders;
        std::unordered_map<GraphicsID, std::string> mArchives;
        std::unordered_map<GraphicsID, size_t> mEntries;
        std::unordered_map<GraphicsID, Surface> mSurfaces;
        GM1::GM1Reader const& GetReader(GraphicsID const&);
        void Load(GraphicsID const&);
        
    public:
        GraphicsManager(Renderer *renderer);
        void SetPaletteIndex(size_t index);
        GraphicsID AddGraphics(std::string, size_t index);
        GraphicsID FindGraphics(std::string const&, size_t index);
        void RemoveGraphics(GraphicsID const&);
        void PaintGraphics(GraphicsID const&);
        void LoadAll(UI::LoadingScreen*);
    };
    
}

#endif