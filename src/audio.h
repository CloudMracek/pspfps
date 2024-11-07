#pragma once

#include "psptypes.h"
#include "stdatomic.h"


typedef struct {
    SceUChar8 mp3Buf[16*1024] __attribute__((aligned(64)));
    SceUChar8 pcmBuf[16*(1152/2)] __attribute__((aligned(64)));
    
    int used;
    int channel;
    int samplingRate;
    int numChannels;
    int lastDecoded;
    int volume;
    int numPlayed;
    int paused;
    int lastButtons;
    int loop;
    int isFinished;
    
    int handle, fd;
    int threadId;
    const char* filename;
} MP3Player;

// Initialize the MP3 library (call once at the start of your application)
void mp3_init();

// Starts an MP3 playback for the specified file with optional looping.
// Returns a unique instance ID for this playback, or -1 if starting fails.
int start_mp3_playback(const char *filename, int loop);

// Stops the MP3 playback for the specified instance ID and cleans up resources.
void stop_mp3_playback(int instanceId);

int is_mp3_playback_finished(int instanceId);

void pause(int instanceId);
void play(int instanceId);

// Pauses playback for the specified MP3 instance.
void mp3_stop(MP3Player *player);

void displayMP3InstancesInfo();

// Fills the MP3 stream buffer for the specified instance (internal use).
int fillStreamBuffer(MP3Player *player);

// Updates MP3 playback for a specific instance (runs in a separate thread).
int mp3_update();

