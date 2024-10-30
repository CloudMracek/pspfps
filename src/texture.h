#pragma once

typedef struct {
    unsigned int width, height;
    unsigned int pW, pH;
    void* data;
    int inVram;
}Texture;


Texture *load_texture(const char *filename, const int vram);

void bind_texture(Texture *tex);

void free_texure(Texture *tex);
