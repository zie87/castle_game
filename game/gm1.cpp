#include "gm1.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/current_function.hpp>

#include <game/endianness.h>

namespace
{

    void Fail(const std::string &where, const std::string &what)
    {
        std::ostringstream oss;
        oss << where << " failed: " << what;
        throw std::runtime_error(oss.str());
    }

}

namespace GM1
{

    std::string GetImageClassName(uint32_t dataClass)
    {
        switch(dataClass) {
        case 1: return "Compressed 16 bit image";
        case 2: return "Compressed animation";
        case 3: return "Tile Object";
        case 4: return "Compressed font";
        case 5: return "Uncompressed bitmap";
        case 6: return "Compressed const size image";
        case 7: return "Uncompressed bitmap (other)";
        default: return "Unknown";
        }
    }
    
    GM1::Encoding GetEncoding(uint32_t dataClass)
    {
        switch(dataClass) {
        case 1: return Encoding::TGX16;
        case 2: return Encoding::TGX8;
        case 3: return Encoding::TileObject;
        case 4: return Encoding::Font;
        case 5: return Encoding::Bitmap;
        case 6: return Encoding::TGX16;
        case 7: return Encoding::Bitmap;
        default: return Encoding::Unknown;
        }
    }

    size_t GetPreambleSize(const GM1::Header &header)
    {
        size_t size = 0;

        /** About 88 bytes on GM1::Header **/
        size += GM1::CollectionHeaderBytes;

        /** About 10 palettes per file 512 bytes each **/
        size += GM1::CollectionPaletteCount * GM1::CollectionPaletteBytes;

        /** 32-bit size per entry **/
        size += header.imageCount * sizeof(uint32_t);

        /** 32-bit offset per entry **/
        size += header.imageCount * sizeof(uint32_t);
        
        /** Some GM1::EntryHeaders of 16 bytes long **/
        size += header.imageCount * GM1::CollectionEntryHeaderBytes;

        return size;
    }

    void PrintEntryHeader(std::ostream &out, const EntryHeader &header)
    {
        using namespace std;
        out << "Width: "        << static_cast<int>(header.width) << endl
            << "Height: "       << static_cast<int>(header.height) << endl
            << "PosX: "         << static_cast<int>(header.posX) << endl
            << "PosY: "         << static_cast<int>(header.posY) << endl
            << "Group: "        << static_cast<int>(header.group) << endl
            << "GroupSize: "    << static_cast<int>(header.groupSize) << endl
            << "TileY: "        << static_cast<int>(header.tileY) << endl
            << "TileOrient: "   << static_cast<int>(header.tileOrient) << endl
            << "Horiz Offset: " << static_cast<int>(header.hOffset) << endl
            << "Box Width: "    << static_cast<int>(header.boxWidth) << endl
            << "Flags: "        << static_cast<int>(header.flags) << endl;
    }

    void PrintHeader(std::ostream &out, const Header &header)
    {
        using namespace std;
        out << "u1: "           << header.u1 << endl
            << "u2: "           << header.u2 << endl
            << "u3: "           << header.u3 << endl
            << "imageCount: "   << header.imageCount << endl
            << "u4: "           << header.u4 << endl
            << "dataClass: "    << GetImageClassName(header.dataClass) << endl
            << "u5: "           << header.u5 << endl
            << "u6: "           << header.u6 << endl
            << "sizeCategory: " << header.sizeCategory << endl
            << "u7: "           << header.u7 << endl
            << "u8: "           << header.u8 << endl
            << "u9: "           << header.u9 << endl
            << "width: "        << header.width << endl
            << "height: "       << header.height << endl
            << "u10: "          << header.u10 << endl
            << "u11: "          << header.u11 << endl
            << "u12: "          << header.u12 << endl
            << "u13: "          << header.u13 << endl
            << "anchorX: "      << header.anchorX << endl
            << "anchorY: "      << header.anchorY << endl
            << "dataSize: "     << header.dataSize << endl
            << "u14: "          << header.u14 << endl;
    }

    void PrintPalette(std::ostream &out, const Palette &palette)
    {
        int column = 0;

        out << std::hex;
        for(auto color : palette) {
            column++;
            
            if(column % 16 == 0)
                out << std::endl;
            
            out << color << ' ';
        }

        out << std::endl;
    }

    SizeCategory ReadSizeCategory(std::istream &in)
    {
        return
            static_cast<SizeCategory>(
                Endian::ReadLittle<uint32_t>(in));
    }
    
    std::ostream& WriteSizeCategory(std::ostream &out, SizeCategory cat)
    {
        return Endian::WriteLittle<uint32_t>(out, static_cast<uint32_t>(cat));
    }   
    
    SizeCategory GetSizeCategoryByDims(int width, int height)
    {
        for(uint32_t n : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}) {
            SizeCategory cat = static_cast<SizeCategory>(n);
            auto dims = GetDimsBySizeCategory(cat);
            if((dims.first == width) && (dims.second == height))
                return cat;
        }
        
        return SizeCategory::Undefined;
    }
    
    std::pair<int, int> GetDimsBySizeCategory(SizeCategory cat)
    {
        switch(cat) {
        case SizeCategory::Undefined:
            break;
        case SizeCategory::Size30x30:
            return std::make_pair(30, 30);
        case SizeCategory::Size55x55:
            return std::make_pair(55, 55);
        case SizeCategory::Size75x75:
            return std::make_pair(75, 75);
        case SizeCategory::Unknown0:
            break;
        case SizeCategory::Size100x100:
            return std::make_pair(100, 100);
        case SizeCategory::Size11x110:
            return std::make_pair(11, 110);
        case SizeCategory::Size130x130:
            return std::make_pair(130, 130);
        case SizeCategory::Unknown1:
            break;
        case SizeCategory::Size185x185:
            return std::make_pair(185, 185);
        case SizeCategory::Size250x250:
            return std::make_pair(250, 250);
        case SizeCategory::Size180x180:
            return std::make_pair(180, 180);
        default:
            break;
        }

        return std::make_pair(0, 0);
    }
    
} // namespace GM1
