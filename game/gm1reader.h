#ifndef GM1READER_H_
#define GM1READER_H_

#include <game/gm1.h>
#include <game/filesystem.h>
#include <game/point.h>

#include <boost/filesystem/fstream.hpp>

class Surface;

namespace GM1
{
    class Palette;
    class GM1EntryReader;
}

namespace GM1
{    
    class GM1Reader
    {
        bool mIsOpened;
        fs::path mPath;
        uint32_t mFlags;
        std::streampos mDataOffset;
        mutable boost::filesystem::ifstream mStream;
        GM1::Header mHeader;
        std::vector<GM1::EntryHeader> mEntryHeaders;
        std::vector<GM1::Palette> mPalettes;
        std::vector<uint32_t> mSizes;
        std::vector<uint32_t> mOffsets;
        std::vector<char> mBuffer;
        mutable std::vector<std::vector<char>> mEntries;
        std::unique_ptr<GM1EntryReader> mEntryReader;
        
    public:
        enum Flags
        {
            NoFlags = 0,
            Cached = 1,
            CheckSizeCategory = 2
        };

        GM1Reader();
        explicit GM1Reader(fs::path, Flags = NoFlags);
        virtual ~GM1Reader();
        
        void Open(fs::path, Flags);
        bool IsOpened() const;
        void Close();
        const Surface ReadEntry(int index) const;
        
        GM1::EntryHeader const& EntryHeader(size_t index) const;
        GM1::Palette const& Palette(size_t index) const;
        GM1::Header const& Header() const;
        GM1::Encoding Encoding() const;
        char const* EntryData(size_t index) const;
        size_t EntrySize(size_t index) const;
        size_t EntryOffset(size_t index) const;
        int NumEntries() const;
        int NumPalettes() const;

        GM1::GM1EntryReader& EntryReader();
    };
}

#endif
