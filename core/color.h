#ifndef COLOR_H_
#define COLOR_H_

#include <iosfwd>
#include <SDL.h>

namespace core
{
    class Color : public SDL_Color
    {
    public:
        constexpr Color();
        constexpr Color(uint8_t r, uint8_t g, uint8_t b);
        constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        constexpr Color(const Color &that);
        constexpr Color(const SDL_Color &that);
        
        constexpr Color Opaque(int alpha) const;

        uint32_t ConvertTo(uint32_t format) const;
        uint32_t ConvertTo(const SDL_PixelFormat &format) const;
    };

    constexpr Color::Color()
        : Color(0, 0, 0, 0)
    {}

    constexpr Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        : SDL_Color {r, g, b, a}
    {}

    constexpr Color::Color(uint8_t r, uint8_t g, uint8_t b)
        : Color(r, g, b, 255)
    {}

    constexpr Color::Color(const Color &that) = default;

    constexpr Color::Color(const SDL_Color &that)
        : Color(that.r, that.g, that.b, that.a)
    {}

    constexpr Color Color::Opaque(int alpha) const
    {
        return Color(r, g, b, alpha);
    }
    
    namespace colors
    {
        constexpr Color Black = Color(0, 0, 0);
        constexpr Color Red = Color(255, 0, 0);
        constexpr Color Green = Color(0, 255, 0);
        constexpr Color Blue = Color(0, 0, 255);
        constexpr Color White = Color(255, 255, 255);
        constexpr Color Magenta = Color(255, 0, 255);
        constexpr Color Yellow = Color(255, 255, 0);
        constexpr Color Cyan = Color(0, 255, 255);
        constexpr Color Gray = Color(128, 128, 128);
    }

    constexpr const Color Inverted(const SDL_Color &color)
    {
        return Color(255 - color.r,
                     255 - color.g,
                     255 - color.b,
                     color.a);
    }

    constexpr bool operator==(const SDL_Color &lhs, const SDL_Color &rhs)
    {
        return (lhs.r == rhs.r)
            && (lhs.g == rhs.g)
            && (lhs.b == rhs.b)
            && (lhs.a == rhs.a);
    }

    constexpr bool operator!=(const SDL_Color &lhs, const SDL_Color &rhs)
    {
        return (lhs.r != rhs.r)
            || (lhs.g != rhs.g)
            || (lhs.b != rhs.b)
            || (lhs.a != rhs.a);
    }

    constexpr bool FullyOpaque(const SDL_Color &color)
    {
        return color.a == 255;
    }

    const Color PixelToColor(uint32_t pixel, uint32_t format);
    const Color PixelToColor(uint32_t pixel, const SDL_PixelFormat &format);

    uint32_t GetPackedPixel(const char *data, int bytesPerPixel);
    void SetPackedPixel(char *data, uint32_t pixel, int bytesPerPixel);
}

std::ostream& operator<<(std::ostream &out, SDL_Color const&);
std::istream& operator>>(std::istream &in, SDL_Color&);
    
#endif // COLOR_H_
