#include <math.h>
#include <pspdisplay.h>
#include <pspgum.h>
#include <string.h>

#include "draw.h"
#include "pspgu.h"
#include "vram.h"

#include "font.c"

#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
static unsigned int __attribute__((aligned(16))) list[262144];

static int fontwidthtab[128] = {
	10, 10, 10, 10, 
	10, 10, 10, 10,
	10, 10, 10, 10, 
	10, 10, 10, 10,

	10, 10, 10, 10, 
	10, 10, 10, 10,
	10, 10, 10, 10,
	10, 10, 10, 10,

	10,  6,  8, 10, //   ! " #
	10, 10, 10,  6, // $ % & '
	10, 10, 10, 10, // ( ) * +
	 6, 10,  6, 10, // , - . /

	10, 10, 10, 10, // 0 1 2 3
	10, 10, 10, 10, // 6 5 8 7
	10, 10,  6,  6, // 10 9 : ;
	10, 10, 10, 10, // < = > ?

	16, 10, 10, 10, // @ A B C
	10, 10, 10, 10, // D E F G
	10,  6,  8, 10, // H I J K
	 8, 10, 10, 10, // L M N O

	10, 10, 10, 10, // P Q R S
	10, 10, 10, 12, // T U V W
	10, 10, 10, 10, // X Y Z [
	10, 10,  8, 10, // \ ] ^ _

	 6,  8,  8,  8, // ` a b c
	 8,  8,  6,  8, // d e f g
	 8,  6,  6,  8, // h i j k
	 6, 10,  8,  8, // l m n o

	 8,  8,  8,  8, // p q r s
	 8,  8,  8, 12, // t u v w
	 8,  8,  8, 10, // x y z {
	 8, 10,  8, 12  // | } ~  
};

float distance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

void draw_string(const char* text, int x, int y, unsigned int color, int fw) {
	int len = (int)strlen(text);
	if(!len) {
		return;
	}

	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	sceGuEnable(GU_BLEND);


	sceGuDisable(GU_DEPTH_TEST);
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, 256, 128, 256, font);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

	typedef struct {
		float s, t;
		unsigned int c;
		float x, y, z;
	} VERT;

	VERT* v = sceGuGetMemory(sizeof(VERT) * 2 * len);

	int i;
	for(i = 0; i < len; i++) {
		unsigned char c = (unsigned char)text[i];
		if(c < 32) {
			c = 0;
		} else if(c >= 128) {
			c = 0;
		}

		int tx = (c & 0x0F) << 4;
		int ty = (c & 0xF0);

		VERT* v0 = &v[i*2+0];
		VERT* v1 = &v[i*2+1];
		
		v0->s = (float)(tx + (fw ? ((16 - fw) >> 1) : ((16 - fontwidthtab[c]) >> 1)));
		v0->t = (float)(ty);
		v0->c = color;
		v0->x = (float)(x);
		v0->y = (float)(y);
		v0->z = 0.0f;

		v1->s = (float)(tx + 16 - (fw ? ((16 - fw) >> 1) : ((16 - fontwidthtab[c]) >> 1)));
		v1->t = (float)(ty + 16);
		v1->c = color;
		v1->x = (float)(x + (fw ? fw : fontwidthtab[c]));
		v1->y = (float)(y + 16);
		v1->z = 0.0f;

		x += (fw ? fw : fontwidthtab[c]);
	}

	sceGumDrawArray(GU_SPRITES, 
		GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
		len * 2, 0, v
	);
	sceGuEnable(GU_DEPTH_TEST);
}

int init_display() {
    void* fbp0 = get_static_vram_buffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	void* fbp1 = get_static_vram_buffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_8888);
	void* zbp = get_static_vram_buffer(BUF_WIDTH,SCR_HEIGHT,GU_PSM_4444);

	sceGuInit();


	sceGuStart(GU_DIRECT,list);
	sceGuDrawBuffer(GU_PSM_8888,fbp0,BUF_WIDTH);
	sceGuDispBuffer(SCR_WIDTH,SCR_HEIGHT,fbp1,BUF_WIDTH);
	sceGuDepthBuffer(zbp,BUF_WIDTH);
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
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	sceGuEnable(GU_LIGHT1);
	sceGuEnable(GU_CLIP_PLANES);

	sceGuFinish();
	sceGuSync(0,0);



	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);


	sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    return 0;
}

void reset_transform(float x, float y, float z) {
	ScePspFVector3 v = {x,y,z};
	sceGumLoadIdentity();
	sceGumTranslate(&v);
}

void reset_rotation(float x, float y, float z) {
	ScePspFVector3 v = {x,y,z};
	sceGumLoadIdentity();
	sceGumRotateXYZ(&v);
}

void start_frame() {
	sceGuStart(GU_DIRECT, list);
}

void end_frame() {
	sceGuFinish();
	sceGuSync(0,0);
	sceDisplayWaitVblankStart();
	sceGuSwapBuffers();
}

void term_graphics() {
	sceGuTerm();
}