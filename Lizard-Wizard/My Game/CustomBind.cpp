#include "CustomBind.h"

// Custom User Input
//NOTE(ethan): We actually only need this function since the keybind and mouse functionality you (me: sean) gave me are identical,
//should be correctly adjusted for the new input system.
void CustomBind::UpdateState() {
    if (GetKeyState(bind) < 0) {
        if (!held) {
            pressed = true;
        }
        else {
            pressed = false;
        }
        held = true;
    }
    else {
        held = false;
        pressed = false;
    }
}
