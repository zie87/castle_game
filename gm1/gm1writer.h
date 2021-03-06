#ifndef gm1WRITER_H_
#define gm1WRITER_H_

#include <iosfwd>

namespace core
{
    class Palette;
}

namespace gm1
{
    class Header;
}

namespace gm1
{
    std::ostream& WriteHeader(std::ostream&, gm1::Header const&);
    std::ostream& WritePalette(std::ostream&, const core::Palette &palette);
}

#endif // gm1WRITER_H_
