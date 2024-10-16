#include "audio.h"
#include "pspdebug.h"
#include "pspiofilemgr.h"
#include "psptypes.h"

#include <stdio.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>
#include <pspiofilemgr_fcntl.h>

#define ERRORMSG(...) { char msg[128]; sprintf(msg,__VA_ARGS__); error(msg); }

void error( char* msg )
{
	pspDebugScreenPrintf(msg, 0, 0, 0xFFFFFF, 0);
	while(1);
}

SceUChar8 	mp3Buf[16*1024]  __attribute__((aligned(64)));
SceUChar8	pcmBuf[16*(1152/2)]  __attribute__((aligned(64)));

    int channel;
	int samplingRate;
	int numChannels;
	int lastDecoded;
	int volume;
	int numPlayed;
	int paused;
	int lastButtons;
	int loop;

    int handle, fd;

int fillStreamBuffer( int fd, int handle )
{
	SceUChar8* dst;
	SceInt32 write;
	SceInt32 pos;
	// Get Info on the stream (where to fill to, how much to fill, where to fill from)
	int status = sceMp3GetInfoToAddStreamData( handle, &dst, &write, &pos);
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3GetInfoToAddStreamData returned 0x%08X\n", status);
	}

	// Seek file to position requested
	status = sceIoLseek32( fd, pos, SEEK_SET );
	if (status<0)
	{
		ERRORMSG("ERROR: sceIoLseek32 returned 0x%08X\n", status);
	}
	
	// Read the amount of data
	int read = sceIoRead( fd, dst, write );
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
	status = sceMp3NotifyAddStreamData( handle, read );
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

    status = sceMp3InitResource();
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3InitResource returned 0x%08X\n", status);
	}
}

void mp3_load(const char* filename) {

    fd = sceIoOpen( filename, PSP_O_RDONLY, 0777);
	if (fd<0)
	{
		ERRORMSG("ERROR: Could not open file '%s' - 0x%08X\n", filename, fd);
	}

    SceMp3InitArg mp3Init;
	mp3Init.mp3StreamStart = 0;
	mp3Init.mp3StreamEnd = sceIoLseek32( fd, 0, SEEK_END );
	mp3Init.mp3Buf = mp3Buf;
	mp3Init.mp3BufSize = sizeof(mp3Buf);
	mp3Init.pcmBuf = pcmBuf;
	mp3Init.pcmBufSize = sizeof(pcmBuf);

    handle = sceMp3ReserveMp3Handle( &mp3Init );
	if (handle<0)
	{
		ERRORMSG("ERROR: sceMp3ReserveMp3Handle returned 0x%08X\n", handle);
	}

    fillStreamBuffer( fd, handle );
	
	int status = sceMp3Init( handle );
	if (status<0)
	{
		ERRORMSG("ERROR: sceMp3Init returned 0x%08X\n", status);
	}

    channel = -1;
	samplingRate = sceMp3GetSamplingRate( handle );
	numChannels = sceMp3GetMp3ChannelNum( handle );
	lastDecoded = 0;
	volume = PSP_AUDIO_VOLUME_MAX;
	numPlayed = 0;
	paused = 0;
	lastButtons = 0;
	loop = 0;
}

int mp3_update(unsigned int arg, void *userData) {
	while(1) {
		if (!paused)
		{
			// Check if we need to fill our stream buffer
			if (sceMp3CheckStreamDataNeeded( handle )>0)
			{
				fillStreamBuffer( fd, handle );
			}

			// Decode some samples
			short* buf;
			int bytesDecoded;
			int retries = 0;
			// We retry in case it's just that we reached the end of the stream and need to loop
			for (;retries<1;retries++)
			{
				bytesDecoded = sceMp3Decode( handle, &buf );
				if (bytesDecoded>0)
					break;
				
				if (sceMp3CheckStreamDataNeeded( handle )<=0)
					break;
				
				if (!fillStreamBuffer( fd, handle ))
				{
					numPlayed = 0;
				}
			}
			if (bytesDecoded<0 && bytesDecoded!=0x80671402)
			{
				ERRORMSG("ERROR: sceMp3Decode returned 0x%08X\n", bytesDecoded);
			}
			
			// Nothing more to decode? Must have reached end of input buffer
			if (bytesDecoded==0 || bytesDecoded==0x80671402)
			{
				paused = 1;
				sceMp3ResetPlayPosition( handle );
				numPlayed = 0;
			}
			else
			{
				// Reserve the Audio channel for our output if not yet done
				if (channel<0 || lastDecoded!=bytesDecoded)
				{
					if (channel>=0)
						sceAudioSRCChRelease();
					
					channel = sceAudioSRCChReserve( bytesDecoded/(2*numChannels), samplingRate, numChannels );
				}
				// Output the decoded samples and accumulate the number of played samples to get the playtime
				numPlayed += sceAudioSRCOutputBlocking( volume, buf );
			}
		}
	}
	return 0;
}


