#include "rendermode.h"

#include <string>
#include <sstream>
#include <memory>
#include <cstring>

#include <boost/filesystem/fstream.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>

#include <gmtool/renderer.h>

#include <game/rect.h>
#include <game/gm1.h>
#include <game/gm1reader.h>
#include <game/palette.h>
#include <game/gm1entryreader.h>
#include <game/image.h>
#include <game/color.h>
#include <game/rw.h>

namespace po = boost::program_options;

namespace gmtool
{
    renderode::~renderode() throw() = default;
    renderode::renderode()
    {
        mFormats = renderormats();
    }
    
    void renderode::GetOptions(po::options_description &opts)
    {
        po::options_description mode("rendermode");
        mode.add_options()
            ("file", po::value(&mInputFile)->required(), "Set gm1 filename")
            ("index,i", po::value(&mEntryIndex)->required(), "Set entry index")
            ("output,o", po::value(&mOutputFile), "Set output image filename")
            ("format,f", po::value(&mFormat)->default_value(mFormats.front().name), "Set rendering format")
            ("palette,p", po::value(&mPaletteIndex), "Set palette index for 8-bit entries")
            ("transparent-color", po::value(&mTransparentColor)->default_value(DefaultTransparent()), "Set background color in #AARRGGBB format")
            ("approximate-size", po::bool_switch(&mApproxSize), "Prints size of resulting image in bytes")
            ;
        opts.add(mode);
    }
    
    void renderode::GetPositionalOptions(po::positional_options_description &unnamed)
    {
        unnamed.add("file", 1);
        unnamed.add("output", 1);
    }

    void renderode::PrintUsage(std::ostream &out)
    {
        out << "Allowed formats are:" << std::endl;
        for(const renderormat &format : mFormats) {
            out.width(3);
            out << ' ';
            out << format.name;
            out << std::endl;
        }
    }

    const core::Color renderode::DefaultTransparent() const
    {
        return core::Color(240, 0, 255, 0);
    }
    
    void renderode::SetupPalette(castle::Image &image, const castle::Palette &palette)
    {
        if(IsPalettized(image)) {
            castle::Palette copied = palette;
            image.AttachPalette(copied);
        }
    }

    void renderode::SetupFormat(castle::Image &image, uint32_t format)
    {
        if(IsPalettized(image)) {
            if(format != image->format->format) {
                image = castle::ConvertImage(image, format);
            }
        }
    }

    void renderode::SetupTransparentColor(castle::Image &surface, const core::Color &color)
    {
        surface.SetColorKey(color);
    }
    
    int renderode::Exec(const ModeConfig &cfg)
    {
        cfg.verbose << "Reading file " << mInputFile << std::endl;
        gm1::gm1Reader reader(mInputFile);

        cfg.verbose << "Collection contains " << reader.NumEntries() << " entries" << std::endl;

        if(mEntryIndex < 0 || mEntryIndex >= reader.NumEntries()) {
            throw std::logic_error("Entry index is out of range");
        }

        if(mPaletteIndex < 0 || mPaletteIndex >= reader.NumPalettes()) {
            throw std::logic_error("Palette index is out of range");
        }

        if(DefaultTransparent() != mTransparentColor) {
            cfg.verbose << "Use transparent: " << mTransparentColor << std::endl;
            //reader.EntryReader().Transparent(mTransparentColor);
        }
        
        castle::Image entry = reader.ReadEntry(mEntryIndex);

        std::ostream *out = nullptr;

        std::ostringstream dummy;
        if(mApproxSize) {
            out = &dummy;
        }

        boost::filesystem::ofstream fout;
        if(!mApproxSize) {
            if(mOutputFile.empty()) {
                throw std::logic_error("You should specify --output option");
            }
            fout.open(mOutputFile, std::ios_base::binary | std::ios_base::out);
            if(!fout) {
                throw std::runtime_error(strerror(errno));
            }
            out = &fout;
        }
        
        cfg.verbose << "Setting up palette" << std::endl;
        SetupPalette(entry, reader.Palette(mPaletteIndex));
        
        cfg.verbose << "Setting up format" << std::endl;
        SetupFormat(entry, gm1::PalettePixelFormat);

        cfg.verbose << "Setting up transparency" << std::endl;
        SetupTransparentColor(entry, mTransparentColor);

        cfg.verbose << "Find appropriate format" << std::endl;
        const renderormat *result = nullptr;
        for(const renderormat &format : mFormats) {
            if(format.name == mFormat)
                result = &format;
        }

        if(result == nullptr) {
            throw std::logic_error("No format with such name");
        }

        cfg.verbose << "Do render" << std::endl;
        result->renderer->renderoStream(*out, entry);

        if(mApproxSize) {
            cfg.verbose << "Printing size" << std::endl;
            out->seekp(0, std::ios_base::end);
            cfg.stdout << out->tellp() << std::endl;
        }
        return EXIT_SUCCESS;
    }
}
