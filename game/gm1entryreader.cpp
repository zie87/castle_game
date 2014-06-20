#include "gm1entryreader.h"

#include <SDL.h>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

#include <game/color.h>
#include <game/rect.h>
#include <game/sdl_utils.h>
#include <game/gm1reader.h>
#include <game/palette.h>
#include <game/gm1.h>
#include <game/tgx.h>
#include <game/image.h>
#include <game/imageview.h>

namespace
{    
    /**
     * \brief Reader for animation sprites.
     *
     * 8-bit images;
     * TGX-compressed;
     */
    class TGX8 : public gm1::gm1EntryReader
    {
    protected:
        uint32_t SourcePixelFormat() const {
            return SDL_PIXELFORMAT_INDEX8;
        }

        void ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const;
    };

    /**
     * \brief Read for static textures.
     *
     * All fonts are such encoded.
     *
     * 16-bit images;
     * TGX-compressed;
     */
    class TGX16 : public gm1::gm1EntryReader
    {
    protected:
        void ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const;
    };

    class FontReader : public gm1::gm1EntryReader
    {
    protected:
        void ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const;
    };
    
    /**
     * \brief Reader for tile textures.
     *
     * Every tile is composed of 30x16 tile rhombus texture and
     * static tgx16 decoration.
     *
     * Tiles are decoded by TGX::DecodeTile
     *
     */
    class TileObject : public gm1::gm1EntryReader
    {
    protected:
        // int Width(const gm1::EntryHeader &header) const {
        //     return gm1::TileSpriteWidth;
        // }
        
        // int Height(const gm1::EntryHeader &header) const {
        //     return gm1::TileSpriteHeight + header.tileY;
        // }
    
        void ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const;
    };

    /**
     * \tile Uncompressed images. 
     */
    class Bitmap : public gm1::gm1EntryReader
    {
    protected:
        int Height(const gm1::EntryHeader &header) const {
            // Nobody knows why
            return header.height - 7;
        }
    
        void ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const;
    };
    
    void TGX8::ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const
    {
        TGX::DecodeImage(in, numBytes, surface);
    }

    void TGX16::ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const
    {
        TGX::DecodeImage(in, numBytes, surface);
    }

    void FontReader::ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const
    {
        TGX::DecodeImage(in, numBytes, surface);

        // Originally I found that just color-keying of an image
        // doesn't work properly. After skipping all fully-transparent
        // pixels of the image some opaque magenta "halo" still remains.

        // In the way to solve it i'd just swapped green channel with alpha
        // and go clean my hands.

        // TODO is there a better way to do so?
        // uint32_t destFormat = SDL_PIXELFORMAT_ARGB8888;
        // castle::Image tmp = castle::ConvertImage(surface, destFormat);

        // uint32_t colorkey = GetColorKey(destFormat);
        // if(SDL_SetColorKey(tmp, SDL_TRUE, colorkey) < 0) {
        //     throw sdl_error();
        // }

        // Here we just ignore original color information. What we are really
        // interested in is green channel
        // auto swap_green_alpha = [](const core::Color &color) {
        //     return core::Color(color.r, 255, color.b, color.g);
        // };
    
        // castle::TransformImage(tmp, swap_green_alpha);

        // surface = tmp;
    }
    
    void Bitmap::ReadImage(std::istream &in, size_t numBytes, gm1::EntryHeader const&, castle::Image &surface) const
    {
        castle::ImageLocker lock(surface);

        const auto stride = surface.RowStride();
        const auto rowBytes = surface.Width() * surface.PixelStride();
        char *const data = lock.Data();

        for(size_t i = 0; (numBytes >= rowBytes) && (i < surface.Height()); ++i) {
            in.read(data + stride * i, rowBytes);
            numBytes -= rowBytes;
        }
    }
    
    // Width of rhombus rows in pixels.
    // \todo should it be placed in separate header file?
    constexpr int PerRow[] = {2, 6, 10, 14, 18, 22, 26, 30, 30, 26, 22, 18, 14, 10, 6, 2};
    
    constexpr int GetTilePixelsPerRow(int row)
    {
        return PerRow[row];
    }

    void ReadTile(std::istream &in, castle::Image &image)
    {
        castle::ImageLocker lock(image);
        
        const auto rowStride = image.RowStride();
        const auto height = gm1::TileSpriteHeight;
        const auto width = gm1::TileSpriteWidth;
        const auto pixelStride = image.PixelStride();
        char *data = lock.Data();
    
        for(size_t y = 0; y < height; ++y) {
            const auto length = GetTilePixelsPerRow(y);
            const auto offset = (width - length) / 2;
            in.read(data + offset * pixelStride, length * pixelStride);
            data += rowStride;
        }
    }
    
    void TileObject::ReadImage(std::istream &in, size_t numBytes, const gm1::EntryHeader &header, castle::Image &surface) const
    {
        const core::Rect tilerect(0, header.tileY, Width(header), gm1::TileSpriteHeight);
        castle::ImageView tile(surface, tilerect);
        ReadTile(in, tile.GetView());
        
        const core::Rect boxrect(header.hOffset, 0, header.boxWidth, Height(header));
        castle::ImageView box(surface, boxrect);
        TGX::DecodeImage(in, numBytes - gm1::TileBytes, box.GetView());
    }
}

namespace gm1
{
    gm1EntryReader::gm1EntryReader()
        : mTransparentColor(255, 0, 255)
    {
    }
    
    castle::Image gm1EntryReader::CreateCompatibleImage(const gm1::EntryHeader &header) const
    {
        const int width = Width(header);
        const int height = Height(header);
        const uint32_t format = SourcePixelFormat();
        
        castle::Image surface = castle::CreateImage(width, height, format);

        surface.SetColorKey(mTransparentColor);

        const uint32_t colorkey = GetColorKey(format);
        if(SDL_FillRect(surface, NULL, colorkey) < 0) {
            throw sdl_error();
        }

        return surface;
    }

    const castle::Image gm1EntryReader::Load(const gm1::EntryHeader &header, const char *data, size_t bytesCount) const
    {
        castle::Image image = CreateCompatibleImage(header);
        boost::iostreams::stream<boost::iostreams::array_source> in(data, bytesCount);
        ReadImage(in, bytesCount, header, image);
        return image;
    }
    
    int gm1EntryReader::Width(const gm1::EntryHeader &header) const
    {
        return header.width;
    }

    int gm1EntryReader::Height(const gm1::EntryHeader &header) const
    {
        return header.height;
    }

    uint32_t gm1EntryReader::SourcePixelFormat() const
    {
        return TGX::PixelFormat;
    }

    uint32_t gm1EntryReader::GetColorKey(uint32_t format) const
    {
        return mTransparentColor.ConvertTo(format);
    }

    const core::Color gm1EntryReader::Transparent() const
    {
        return mTransparentColor;
    }

    void gm1EntryReader::Transparent(core::Color color)
    {
        mTransparentColor = std::move(color);
    }
    
    gm1EntryReader::Ptr CreateEntryReader(const Encoding &encoding)
    {
        switch(encoding) {
        case Encoding::Font:
            return gm1EntryReader::Ptr(new FontReader);
            
        case Encoding::TGX16:
            return gm1EntryReader::Ptr(new TGX16);
                
        case Encoding::Bitmap:
            return gm1EntryReader::Ptr(new Bitmap);
        
        case Encoding::TGX8:
            return gm1EntryReader::Ptr(new TGX8);
            
        case Encoding::TileObject:
            return gm1EntryReader::Ptr(new TileObject);
            
        case Encoding::Unknown:
        default:
            throw std::runtime_error("Unknown encoding");
        }
    }
}
