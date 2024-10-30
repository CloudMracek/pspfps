#pragma once

int init_display();
void start_frame();
void end_frame();
void term_graphics();

void reset_transform(float x, float y, float z);
void reset_rotation(float x, float y, float z);

float distance3D(float x1, float y1, float z1, float x2, float y2, float z2);

void draw_string(const char *text, int x, int y, unsigned int color, int fw);
