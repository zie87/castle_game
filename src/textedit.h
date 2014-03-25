#ifndef TEXTEDIT_H_
#define TEXTEDIT_H_

#include "widget.h"

namespace Castle
{
    class Engine;
}

namespace UI
{

    class TextEdit : public Widget
    {
    public:
        TextEdit(Castle::Engine *engine);
    };
    
}

#endif
