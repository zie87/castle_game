#ifndef gm1ENTRYREADER_H_
#define gm1ENTRYREADER_H_

#include <iosfwd>
#include <memory>

#include <game/color.h>

namespace gm1
{
    class gm1Reader;
    class EntryHeader;
    class Header;
    enum class Encoding;
}

namespace castle
{
    class Image;
}

namespace gm1
{
    class gm1EntryReader
    {
        core::Color mTransparentColor;

    private:
        castle::Image CreateCompatibleImage(const gm1::EntryHeader &header) const;
        uint32_t GetColorKey(uint32_t format) const;
        
    protected:
        virtual void ReadImage(std::istream &in, size_t numBytes, const gm1::EntryHeader &header, castle::Image &surface) const = 0;
        virtual int Width(const gm1::EntryHeader &header) const;
        virtual int Height(const gm1::EntryHeader &header) const;
        virtual uint32_t SourcePixelFormat() const;
        
    public:
        gm1EntryReader();
        virtual ~gm1EntryReader() = default;
        
        void Transparent(core::Color color);
        const core::Color Transparent() const;
        const castle::Image Load(const gm1::EntryHeader &header, const char *data, size_t bytesCount) const;

        typedef std::unique_ptr<gm1EntryReader> Ptr;
    };
    
    gm1EntryReader::Ptr CreateEntryReader(gm1::Encoding const&);
}

#endif
