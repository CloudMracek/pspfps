#pragma once

int init_display();
void start_frame();
void end_frame();
void term_graphics();

void reset_transform(float x, float y, float z);
void reset_rotation(float x, float y, float z);

void draw_string(const char* text, int x, int y, unsigned int color, int fw);
