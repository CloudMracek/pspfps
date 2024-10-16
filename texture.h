#pragma once

typedef struct {
    unsigned int width, height;
    unsigned int pW, pH;
    void* data;
}Texture;


Texture *load_texture(const char *filename, const int vram);
