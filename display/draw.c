#include <pspdisplay.h>

#include "draw.h"
#include "../types.h"
#include "pspgu.h"
#include "vram.h"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

void* dual_buffers[2];
void* z_buffer;

static unsigned int __attribute__((aligned(16))) list[262144];

int init_display() {
    dual_buffers[0] = get_static_vram_buffer(SCR_WIDTH, SCR_HEIGHT, GU_PSM_8888);
    dual_buffers[1] = get_static_vram_buffer(SCR_WIDTH, SCR_HEIGHT, GU_PSM_8888);
    z_buffer = get_static_vram_buffer(SCR_WIDTH, SCR_HEIGHT, GU_PSM_4444);

    sceGuInit();

    sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,dual_buffers[0],BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,dual_buffers[1],BUF_WIDTH);
	sceGuDepthBuffer(z_buffer,BUF_WIDTH);
	sceGuOffset(2048 - (SCR_WIDTH/2),2048 - (SCR_HEIGHT/2));
	sceGuViewport(2048,2048,SCR_WIDTH,SCR_HEIGHT);
	sceGuDepthRange(65535,0);
	sceGuScissor(0,0,SCR_WIDTH,SCR_HEIGHT);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuEnable(GU_CLIP_PLANES);
	sceGuFinish();
	sceGuSync(0,0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);
    return 0;
}