#ifndef CUSTOMBIND_H
#define CUSTOMBIND_H

#include "Defines.h"

// Bind Struct
struct CustomBind {
    i32 bind;
    b8 held, pressed;

    CustomBind(i32 bind);
    void UpdateState();

private:
    b8 _pad0, _pad1;
};

CustomBind::CustomBind(i32 bind) {
    this->bind = bind;
    held = 0;
    pressed = 0;
    _pad0 = 0;
    _pad1 = 0;
}

#endif
