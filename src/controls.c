#include <pspctrl.h>
#include "controls.h"

SceCtrlData pad;
SceCtrlData prevPad;

void init_controls() {
    sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

void update_controls() {
    prevPad = pad;
    sceCtrlReadBufferPositive(&pad, 1);
}

int get_button(enum PspCtrlButtons button) {
    return pad.Buttons & button;
}

int get_button_down(enum PspCtrlButtons button) {
    if(pad.Buttons & button && !(prevPad.Buttons & button)) {
        return 1;
    }
    else {
        return 0;
    }
}

float get_Lx() {
    return pad.Lx;
}

float get_Ly() {
    return pad.Ly;
}