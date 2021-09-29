#ifndef CUSTOMBIND_H
#define CUSTOMBIND_H

#include "Defines.h"

// Bind Struct
struct CustomBind {
    i32 bind;
    b8 held, pressed;

    // we make this static so the compiler doesnt complain when we dont have a default constructor
    static CustomBind New(i32 bind);
    void UpdateState();

private:
    b8 _pad0, _pad1;
};

#endif
