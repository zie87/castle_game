#ifndef TEXTBOX_H_
#define TEXTBOX_H_

#include "widget.h"

class Text;
class Engine;

class TextBox : public Widget
{
public:
    TextBox(Engine *engine);
    void SetText(const Text &text);
};


#endif
