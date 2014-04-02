#ifndef SCROLLBAR_H_
#define SCROLLBAR_H_

#include <memory>
#include "widget.h"
#include "button.h"

namespace UI
{
    
    class ScrollBar : public Widget
    {
    public:
        ScrollBar(Castle::Engine *engine);
    };
    
}

#endif