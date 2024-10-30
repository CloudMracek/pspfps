#pragma once

void mp3_stop();

void mp3_init();

void mp3_load(const char *filename, int loop);

int mp3_update(unsigned int arg, void *userData);
