#include "tgx.h"

int PopCount(Uint8 n)
{
    int count = 0;
    while(n != 0) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}

bool CheckTokenType(const TGXToken &token)
{
    // There should be only single non-zero bit.
    return (PopCount(token >> 5) <= 1);
}

bool CheckTokenLength(const TGXToken &token)
{
    // Length for Newline token should be always equal to 1
    return (GetTokenType(token) != TokenType::Newline)
        || (GetTokenLength(token) == 1);
}

Sint64 GetAvailableBytes(SDL_RWops *src)
{
    return SDL_RWsize(src) - SDL_RWseek(src, 0, RW_SEEK_CUR);
}

bool CheckBytesAvailable(SDL_RWops *src, Sint64 bytes) throw()
{
    return (GetAvailableBytes(src) >= bytes);
}

bool CheckTGXSize(Uint32 width, Uint32 height)
{
    return (width <= MAX_TGX_WIDTH) && (height <= MAX_TGX_HEIGHT);
}

template<class P>
void ReadPixelArray(SDL_RWops *src, P *buffer, Uint32 count);

template<>
void ReadPixelArray(SDL_RWops *src, Uint16 *buffer, Uint32 count)
{
    // if(!CheckBytesAvailable(src, sizeof(Uint16) * count))
    //     throw EOFError("ReadPixelArray<Uint16>");

    SDL_RWread(src, buffer, count, sizeof(Uint16));

    if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        for(Uint16 *p = buffer; p != buffer + count; ++p)
            *p = SDL_Swap16(*p);
    }
}

template<>
void ReadPixelArray(SDL_RWops *src, Uint8 *buffer, Uint32 count)
{
    // if(!CheckBytesAvailable(src, sizeof(Uint8) * count))
    //     throw EOFError("ReadPixelArray<Uint8>");

    SDL_RWread(src, buffer, count, sizeof(Uint8));
}

template<class P>
void ReadTGXImage(SDL_RWops *src, Uint32 size, Uint32 width, Uint32 height, P *pImg)
{
    TGXToken token;
    
    auto pStart = pImg;
    auto pEnd = pStart + width * height;

    if(!CheckTGXSize(width, height))
        throw FormatError("Image too big");
    
    auto origin = SDL_RWseek(src, 0, RW_SEEK_CUR);
    while(SDL_RWseek(src, 0, RW_SEEK_CUR) < origin + size) {
        ReadTGXToken(src, &token);
        
        if(!CheckTokenType(token)) {
            SDL_LogDebug(SDL_LOG_PRIORITY_WARN, "Wrong TGXToken type %d",
                         token);
            throw FormatError("Wrong token type");
        }
        
        if(!CheckTokenLength(token))
            throw FormatError("Wrong token length");

        if(GetTokenType(token) != TokenType::Newline) {
            if(pImg + GetTokenLength(token) > pEnd)
                throw FormatError("Buffer overflow");
        }
        
        switch(GetTokenType(token)) {
        case TokenType::Stream:
            ReadPixelArray<P>(src, pImg, GetTokenLength(token));
            pImg += GetTokenLength(token);
            break;
            
        case TokenType::Repeat: {
            P pixel;
            ReadPixelArray<P>(src, &pixel, 1);
            std::fill(pImg, pImg + GetTokenLength(token), pixel);
            pImg += GetTokenLength(token);
            break;
        }
            
        case TokenType::Transparent:
            pImg += GetTokenLength(token);
            break;
            
        case TokenType::Newline:
            if((pImg - pStart) % width != 0)
                pImg += width - (pImg - pStart) % width - 1;
            break;
        }
    }
}

void ReadTGXImage16(SDL_RWops *src, Uint32 size, Uint32 width, Uint32 height, Uint16 *pImg)
{
    ReadTGXImage<Uint16>(src, size, width, height, pImg);
}

void ReadTGXImage8(SDL_RWops *src, Uint32 size, Uint32 width, Uint32 height, Uint8 *pImg)
{
    ReadTGXImage<Uint8>(src, size, width, height, pImg);
}

void ReadTGXHeader(SDL_RWops *src, TGXHeader *hdr)
{
    if(!CheckBytesAvailable(src, sizeof(TGXHeader)))
        throw EOFError("ReadTGXHeader");
    
    hdr->width = SDL_ReadLE32(src);
    hdr->height = SDL_ReadLE32(src);
}    

void ReadTGXToken(SDL_RWops *src, TGXToken *token)
{
    // Very slow
    // if(!CheckBytesAvailable(src, sizeof(TGXToken)))
    //      throw EOFError("ReadTGXToken");
    SDL_RWread(src, token, 1, sizeof(TGXToken));
}

void ReadBitmap(SDL_RWops *src, Uint32 size, Uint16 *pixels)
{
    ReadPixelArray<Uint16>(src, pixels, size / sizeof(Uint16));
}

void ReadTile(SDL_RWops *src, Uint16 *pixels)
{
    for(size_t row = 0; row < TILE_RHOMBUS_HEIGHT; ++row) {
        size_t offset = (TILE_RHOMBUS_WIDTH - TILE_PIXELS_PER_ROW[row]) / 2;
        ReadPixelArray<Uint16>(src, pixels + offset, TILE_PIXELS_PER_ROW[row]);
        pixels += TILE_RHOMBUS_WIDTH;
    }
}

SDL_Surface * CreateRGBSurfaceFromTGX16(Uint16 *buff, Uint32 width, Uint32 height)
{
    return SDL_CreateRGBSurfaceFrom(
        buff,
        width, height, sizeof(Uint16) * 8,
        width * sizeof(Uint16),
        TGX_RGB16_RMASK,
        TGX_RGB16_GMASK,
        TGX_RGB16_BMASK,
        TGX_RGB16_AMASK);
}
