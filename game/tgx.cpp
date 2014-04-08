#include "tgx.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <SDL.h>

#include <boost/current_function.hpp>

#include <game/endianness.h>
#include <game/surface.h>

namespace
{
    
    struct Header
    {
        uint32_t width;
        uint32_t height;
    };

    /**
       TGX is just the sequence of tokens.
    **/
    enum class TokenType : int
    {
        Stream = 0,
        Transparent = 1,
        Repeat = 2,
        LineFeed = 4
    };

    typedef uint8_t token_t;

    const int MaxTokenLength = 32;

    /**
     * Each token has its length. It is represented by lower 5 bits.
     * \note There are no 0 length.
     */
    constexpr size_t ExtractTokenLength(token_t token)
    {
        return (token & 0x1f) + 1;
    }

    /**
     * Each token has its type which are of type TokenType.
     * It is higher 3 bits.
     */
    constexpr TokenType ExtractTokenType(token_t token)
    {
        return static_cast<TokenType>(token >> 5);
    }

    /**
     * Given type and length produce token. Token may be invalid since
     * its length remains unchecked
     */
    constexpr token_t MakeToken(TokenType type, int length)
    {
        return ((static_cast<int>(type) & 0x0f) << 5) | ((length - 1) & 0x1f);
    }

    constexpr token_t MakeStreamToken(int length)
    {
        return MakeToken(TokenType::Stream, std::move(length));
    }

    constexpr token_t MakeRepeatToken(int length)
    {
        return MakeToken(TokenType::Repeat, std::move(length));
    }

    constexpr token_t MakeTransparentToken(int length)
    {
        return MakeToken(TokenType::Transparent, std::move(length));
    }

    constexpr token_t MakeLineFeedToken()
    {
        return MakeToken(TokenType::LineFeed, 1);
    }
    
    std::string GetTokenTypeName(TokenType type)
    {
        switch(type) {
        case TokenType::Transparent: return "Transparent";
        case TokenType::Stream: return "Stream";
        case TokenType::LineFeed: return "LineFeed";
        case TokenType::Repeat: return "Repeat";
        default: return "Unknown";
        }
    }
        
    void Fail(const std::string &where, const std::string &what)
    {
        std::ostringstream oss;
        oss << where << " failed: " << what;
        throw std::runtime_error(oss.str());
    }
    
    std::istream& ReadHeader(std::istream &in, Header &header)
    {
        header.width = Endian::ReadLittle<uint32_t>(in);
        header.height = Endian::ReadLittle<uint32_t>(in);
        return in;
    }
    
    Surface CreateCompatibleSurface(int width, int height, int bpp)
    {
        uint32_t rmask = DefaultRedMask;
        uint32_t gmask = DefaultGreenMask;
        uint32_t bmask = DefaultBlueMask;
        uint32_t amask = DefaultAlphaMask;
        
        if(bpp == 16) {
            rmask = TGX::RedMask16;
            gmask = TGX::GreenMask16;
            bmask = TGX::BlueMask16;
            amask = TGX::AlphaMask16;
        }
    
        Surface surface = SDL_CreateRGBSurface(NoFlags, width, height, bpp, rmask, gmask, bmask, amask);
        if(surface.Null()) {
            Fail(BOOST_CURRENT_FUNCTION, SDL_GetError());
        }
        return surface;
    }
    
    void RepeatPixel(const char *pixel, char *buff, size_t size, size_t count)
    {
        for(size_t i = 0; i < count; ++i) {
            std::copy(pixel, pixel + size, buff + size * i);
        }
    }

    bool PixelsEqual(const char *lhs, const char *rhs, int bytesPerPixel)
    {
        return std::equal(lhs, lhs + bytesPerPixel, rhs);
    }

    int PixelsCount(const char *lhs, const char *rhs, int bytesPerPixel)
    {
        return std::distance(lhs, rhs) / bytesPerPixel;
    }

    bool PixelTransparent(const char *pixels, int bytesPerPixel)
    {
        const uint16_t *colors = reinterpret_cast<uint16_t const*>(pixels);
        return (bytesPerPixel == 2) && (*colors == TGX::Transparent16);
    }
    
    std::ostream& WriteStreamToken(std::ostream &out, const char *pixels, int numPixels, int bytesPerPixel)
    {
        Endian::WriteLittle<uint8_t>(out, MakeStreamToken(numPixels));
        return out.write(pixels, numPixels * bytesPerPixel);
    }

    std::ostream& WriteLineFeed(std::ostream &out)
    {
        return Endian::WriteLittle<uint8_t>(out, MakeLineFeedToken());
    }

    std::ostream& WriteTransparentToken(std::ostream &out, int numPixels)
    {
        return Endian::WriteLittle<uint8_t>(out, MakeTransparentToken(numPixels));
    }
    std::ostream& WriteRepeatToken(std::ostream &out, const char *pixels, int numPixels, int bytesPerPixel)
    {
        // We just ignore endianess since pixels are forced to be LE
        if(PixelTransparent(pixels, bytesPerPixel)) {
            return WriteTransparentToken(out, numPixels);
        }
        
        Endian::WriteLittle<uint8_t>(out, MakeRepeatToken(numPixels));
        return out.write(pixels, bytesPerPixel);
    }
    
}

namespace TGX
{
    
    std::ostream& WriteTGX(std::ostream &out, int width, int height, const char *data, size_t numBytes)
    {
        Endian::WriteLittle<uint32_t>(out, width);
        Endian::WriteLittle<uint32_t>(out, height);
        return out.write(data, numBytes);
    }

    std::ostream& EncodeBuffer(std::ostream &out, const char *pixels, int width, int bytesPerPixel)
    {
        const char *end = pixels + width * bytesPerPixel;
        const char *streamStart = pixels;
        const char *repeatStart = pixels;

        while(pixels != end) {
            assert((pixels - streamStart) <= MaxTokenLength);
            assert((pixels - repeatStart) <= MaxTokenLength);

            const int numStream = PixelsCount(streamStart, pixels, bytesPerPixel);
            if(numStream == MaxTokenLength) {
                if(streamStart != repeatStart) {
                    const int numStreamBeforeRepeat = PixelsCount(streamStart, repeatStart, bytesPerPixel);
                    WriteStreamToken(out, streamStart, numStreamBeforeRepeat, bytesPerPixel);
                    streamStart = repeatStart;
                } else {
                    WriteRepeatToken(out, streamStart, numStream, bytesPerPixel);
                    repeatStart = pixels;
                    streamStart = pixels;
                }
            }

            if(!PixelsEqual(repeatStart, pixels, bytesPerPixel)) {
                const int numRepeat = PixelsCount(repeatStart, pixels, bytesPerPixel);
                if(numRepeat >= 2) {
                    WriteRepeatToken(out, repeatStart, numRepeat, bytesPerPixel);
                }
                repeatStart = pixels;
            }

            pixels += bytesPerPixel;
        }

        const int numStream = PixelsCount(streamStart, pixels, bytesPerPixel);
        const int numRepeat = PixelsCount(repeatStart, pixels, bytesPerPixel);

        if(numRepeat >= 2) {
            if(numStream > numRepeat) {
                const int numStreamBeforeRepeat = PixelsCount(streamStart, repeatStart, bytesPerPixel);
                WriteStreamToken(out, streamStart, numStreamBeforeRepeat, bytesPerPixel);
            }
            WriteRepeatToken(out, repeatStart, numRepeat, bytesPerPixel);
        } else if(numStream != 0) {
            WriteStreamToken(out, streamStart, numStream, bytesPerPixel);
        }
                
        return WriteLineFeed(out);
    }
    
    std::ostream& EncodeSurface(std::ostream &out, const Surface &surface)
    {
        SurfaceLocker lock(surface);
        const char *pixelsPtr = ConstGetPixels(surface);
        const int bytesPerPixel = surface->format->BytesPerPixel;

        for(int row = 0; row < surface->h; ++row) {
            if(!EncodeBuffer(out, pixelsPtr, surface->w, bytesPerPixel)) {
                break;
            }
            pixelsPtr += surface->pitch;
        }
            
        return out;
    }

    Surface ReadTGX(std::istream &in)
    {
        Header header;
        if(!ReadHeader(in, header)) {
            Fail(BOOST_CURRENT_FUNCTION, "Can't read header");
        }
        Surface surface = CreateCompatibleSurface(header.width, header.height, 16);

        const std::streampos origin = in.tellg();
        in.seekg(0, std::ios_base::end);
        const std::streampos fsize = in.tellg();
        in.seekg(origin);
        
        TGX::DecodeSurface(in, fsize - origin, surface);
        return surface;
    }

    std::istream& DecodeBuffer(std::istream &in, size_t numBytes, char *dst, size_t width, size_t bytesPerPixel)
    {
        const std::streampos endPos = numBytes + in.tellg();
        const char *dstEnd = dst + width * bytesPerPixel;
        
        while(in.tellg() < endPos) {
            const token_t token = Endian::ReadLittle<token_t>(in);
            // \note any io errors are just ignored if token has valid TokenType

            const TokenType type = ExtractTokenType(token);
            const int length = ExtractTokenLength(token);
            
            switch(type) {
            case TokenType::Repeat:
            case TokenType::Stream:
            case TokenType::Transparent:
                {
                    // \todo what if new dst value is equal to dstEnd? In that case
                    // we have no space for placing LineFeed. It is certainly an erroneous behavior.
                    // Should we report it here?
                    if(dst + length * bytesPerPixel > dstEnd) {
                        Fail(BOOST_CURRENT_FUNCTION, "Overflow");
                    }
                }
            default:
                break;
            }
            
            switch(type) {
            case TokenType::LineFeed:
                {
                    if(length != 1) {
                        Fail(BOOST_CURRENT_FUNCTION, "Inconsistent line feed");
                    }
                    return in;
                }
                break;
                
            case TokenType::Repeat:
                {
                    in.read(dst, bytesPerPixel);
                    for(int n = 0; n < length; ++n) {
                        std::copy(dst, dst + bytesPerPixel, dst + n * bytesPerPixel);
                    }
                }
                break;
                
            case TokenType::Stream:
                {
                    in.read(dst, length * bytesPerPixel);
                }
                break;
                
            case TokenType::Transparent:
                break;
                
            default:
                {
                    if(!in) {
                        Fail(BOOST_CURRENT_FUNCTION, "Unable to read token");
                    }
                    Fail(BOOST_CURRENT_FUNCTION, "Unknown token");
                }
            }

            if(!in) {
                Fail(BOOST_CURRENT_FUNCTION, "Overrun");
            }
            
            dst += length * bytesPerPixel;
        }
        
        return in;
    }
    
    void DecodeSurface(std::istream &in, size_t numBytes, Surface &surface)
    {
        const SurfaceLocker lock(surface);

        const int width = surface->w;
        const int height = surface->h;
        const int bytesPP = surface->format->BytesPerPixel;
        const int pitch = surface->pitch;

        const std::streampos endPos = numBytes + in.tellg();
        
        char *dst = GetPixels(surface);

        for(int row = 0; row < height; ++row) {
            if((in) && (in.tellg() < endPos)) {
                size_t bytesLeft = endPos - in.tellg();
                DecodeBuffer(in, bytesLeft, dst, width, bytesPP);
                dst += pitch;
            }
        }

        // Make a look that we read all numBytes
        if(in) {
            in.seekg(endPos);
        }
    }
    
} // namespace TGX
