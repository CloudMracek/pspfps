#pragma once
#include <pspctrl.h>

void init_controls();
void update_controls();
int get_button_down(enum PspCtrlButtons button);
int get_button(enum PspCtrlButtons button);
float get_Lx();
float get_Ly();
