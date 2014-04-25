#ifndef HEADERMODE_H_
#define HEADERMODE_H_

#include "mode.h"

#include <boost/filesystem/path.hpp>
#include <iostream>

namespace GMTool
{
    class HeaderMode : public Mode
    {
        boost::filesystem::path mInputFile;
        bool mBinary = false;
        bool mCountRequested = false;
        bool mEncodingRequested = false;
    public:
        void GetOptions(boost::program_options::options_description&);
        void GetPositionalOptions(boost::program_options::positional_options_description&);
        int Exec(const ModeConfig &config);
    };
}

#endif // HEADERMODE_H_
