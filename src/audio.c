#include "audio.h"
#include "pspdebug.h"
#include "pspiofilemgr.h"
#include "psploadexec.h"
#include "pspthreadman.h"

#include <stdio.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>
#include <pspiofilemgr_fcntl.h>

#define ERRORMSG(...) { char msg[128]; sprintf(msg,__VA_ARGS__); error(msg); }
#define MAX_MP3_INSTANCES 4  // Static number of MP3 instances



static MP3Player mp3Instances[MAX_MP3_INSTANCES];

void error(char* msg) {
    pspDebugScreenPrintf(msg, 0, 0, 0xFFFFFF, 0);
}

int fillStreamBuffer(MP3Player *player) {
    unsigned char* dst;
	long int write;
	long int pos;
	// Get Info on the stream (where to fill to, how much to fill, where to fill from)
	int status = sceMp3GetInfoToAddStreamData( player->handle, &dst, &write, &pos);
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3GetInfoToAddStreamData returned 0x%08X\n", status);
	}

	// Seek file to position requested
	status = sceIoLseek32( player->fd, pos, SEEK_SET );
	if (status<0)
	{
		ERRORMSG("ERROR: sceIoLseek32 returned 0x%08X\n", status);
	}
	
	// Read the amount of data
	int read = sceIoRead( player->fd, dst, write );
	if (read < 0)
	{
		ERRORMSG("ERROR: Could not read from file - 0x%08X\n", read);
	}
	
	if (read==0)
	{
		// End of file?
		return 0;
	}
	
	// Notify mp3 library about how much we really wrote to the stream buffer
	status = sceMp3NotifyAddStreamData( player->handle, read );
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3NotifyAddStreamData returned 0x%08X\n", status);
	}
	
	return (pos>0);
}

void mp3_init() {
   int status = sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	if (status<0)
	{
		ERRORMSG("ERROR: sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC) returned 0x%08X\n", status);
	}
	
	status = sceUtilityLoadModule(PSP_MODULE_AV_MP3);
	if (status<0)
	{
		ERRORMSG("ERROR: sceUtilityLoadModule(PSP_MODULE_AV_MP3) returned 0x%08X\n", status);
	}
	// Init mp3 resources
	status = sceMp3InitResource();
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3InitResource returned 0x%08X\n", status);
	}

}



void mp3_load(MP3Player *player, const char* filename, int loop) {
	
    player->fd = sceIoOpen(filename, PSP_O_RDONLY, 0777);
    if (player->fd < 0) {
        ERRORMSG("ERROR: Could not open file '%s' - 0x%08X\n", filename, player->fd);
    }

    SceMp3InitArg mp3Init;
    mp3Init.mp3StreamStart = 0;
    mp3Init.mp3StreamEnd = sceIoLseek32(player->fd, 0, SEEK_END);
    mp3Init.mp3Buf = player->mp3Buf;
    mp3Init.mp3BufSize = sizeof(player->mp3Buf);
    mp3Init.pcmBuf = player->pcmBuf;
    mp3Init.pcmBufSize = sizeof(player->pcmBuf);

    player->handle = sceMp3ReserveMp3Handle(&mp3Init);
    if (player->handle < 0) {
        ERRORMSG("ERROR: sceMp3ReserveMp3Handle returned 0x%08X\n", player->handle);
    }

    fillStreamBuffer(player);

    int status = sceMp3Init(player->handle);
    if (status < 0) {
        ERRORMSG("ERROR: sceMp3Init returned 0x%08X\n", status);
    }
	
	player->used = 1;
    player->channel = -1;
    player->samplingRate = sceMp3GetSamplingRate(player->handle);
    player->numChannels = sceMp3GetMp3ChannelNum(player->handle);
    player->lastDecoded = 0;
    player->volume = PSP_AUDIO_VOLUME_MAX;
    player->numPlayed = 0;
    player->paused = 0;
    player->lastButtons = 0;
	player->loop = loop;

	status = sceMp3SetLoopNum( player->handle, loop );
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3SetLoopNum returned 0x%08X\n", status);
	}
	
}

int mp3_update() {
	for(int i = 0; i < 4; i++) {
    MP3Player* player = &mp3Instances[i];
	if(player->used == 0) continue;
		if (!player->paused)
		{	
			pspDebugScreenPrintf("lol", 0, 0, 0xFFFFFF, 0);
			// Check if we need to fill our str	am buffer
			if (sceMp3CheckStreamDataNeeded( player->handle )>0)
			{
				fillStreamBuffer( player );
			}

			pspDebugScreenPrintf("lol", 0, 20, 0xFFFFFF, 0);
			// Decode some samples
			short* buf;
			int bytesDecoded;
			int retries = 0;
			// We retry in case it's just that we reached the end of the stream and need to loop
							pspDebugScreenPrintf("lol", 0,30, 0xFFFFFF, 0);

			for (;retries<1;retries++)
			{	
				bytesDecoded = sceMp3Decode( player->handle, &buf );
				if (bytesDecoded>0)
					break;
				
				if (sceMp3CheckStreamDataNeeded( player->handle )<=0)
					break;
				
				if (!fillStreamBuffer( player ))
				{
					player->numPlayed = 0;
				}
			}
							pspDebugScreenPrintf("lol", 0,40, 0xFFFFFF, 0);

			if (bytesDecoded<0 && bytesDecoded!=0x80671402)
			{
				ERRORMSG("ERROR: sceMp3Decode returned 0x%08X\n", bytesDecoded);
			}
			
			// Nothing more to decode? Must have reached end of input buffer
			if (bytesDecoded==0 || bytesDecoded==0x80671402)
			{
				player->paused = 1;
				sceMp3ResetPlayPosition( player->handle );
				player->numPlayed = 0;
				stop_mp3_playback(i);
			}
			else
			{
			
				// Reserve the Audio channel for our output if not yet done
				if (player->channel<0 || player->lastDecoded!=bytesDecoded)
				{
					if (player->channel>=0)
						sceAudioSRCChRelease();
					
					player->channel = sceAudioSRCChReserve( bytesDecoded/(2*player->numChannels), player->samplingRate, player->numChannels );
				}
				// Output the decoded samples and accumulate the number of played samples to get the playtime
				player->numPlayed += sceAudioSRCOutputBlocking( player->volume, buf );
				player->lastDecoded = bytesDecoded;
			}

		}
	}
	
	
}

int start_mp3_playback(const char* filename, int loop) {
    for (int i = 0; i < MAX_MP3_INSTANCES; i++) {
        if (mp3Instances[i].used == 0) {
            mp3_load(&mp3Instances[i], filename, loop);

            return i;
        }
    }
    return -1;
}

void stop_mp3_playback(int instanceId) {
    if (instanceId >= 0 && instanceId < MAX_MP3_INSTANCES && mp3Instances[instanceId].used != 0) {
        sceKernelTerminateDeleteThread(mp3Instances[instanceId].threadId);
        sceMp3ReleaseMp3Handle(mp3Instances[instanceId].handle);
        sceIoClose(mp3Instances[instanceId].fd);
        mp3Instances[instanceId].used = 0;
    }
}

int is_mp3_playback_finished(int instanceId) {
    if (instanceId < 0 || instanceId >= MAX_MP3_INSTANCES) {
        return 1; // Invalid instance ID; treat as "finished"
    }
    MP3Player *player = &mp3Instances[instanceId];
    return player->used? 0 : 1;
}
