#ifndef GM1ENTRYREADER_H_
#define GM1ENTRYREADER_H_

#include <iosfwd>
#include <memory>

#include <game/color.h>

namespace GM1
{
    class GM1Reader;
    class EntryHeader;
    class Header;
    enum class Encoding;
}

namespace Castle
{
    class Image;
}

namespace GM1
{
    class GM1EntryReader
    {
        core::Color mTransparentColor;

    private:
        Castle::Image CreateCompatibleImage(const GM1::EntryHeader &header) const;
        uint32_t GetColorKey(uint32_t format) const;
        
    protected:
        virtual void ReadImage(std::istream &in, size_t numBytes, const GM1::EntryHeader &header, Castle::Image &surface) const = 0;
        virtual int Width(const GM1::EntryHeader &header) const;
        virtual int Height(const GM1::EntryHeader &header) const;
        virtual uint32_t SourcePixelFormat() const;
        
    public:
        GM1EntryReader();
        virtual ~GM1EntryReader() = default;
        
        void Transparent(core::Color color);
        const core::Color Transparent() const;
        const Castle::Image Load(const GM1::EntryHeader &header, const char *data, size_t bytesCount) const;

        typedef std::unique_ptr<GM1EntryReader> Ptr;
    };
    
    GM1EntryReader::Ptr CreateEntryReader(GM1::Encoding const&);
}

#endif
