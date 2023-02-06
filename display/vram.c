#include <pspge.h>
#include <pspgu.h>

static unsigned int staticOffset = 0;

static unsigned int get_mem_size(unsigned int width, unsigned int height, unsigned int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

void* get_static_vram_buffer(unsigned int width, unsigned int height, unsigned int psm)
{
	unsigned int memSize = get_mem_size(width,height,psm);
	void* result = (void*)staticOffset;
	staticOffset += memSize;

	return result;
}

void* get_static_vram_texture(unsigned int width, unsigned int height, unsigned int psm)
{
	void* result = get_static_vram_buffer(width,height,psm);
	return (void*)(((unsigned int)result) + ((unsigned int)sceGeEdramGetAddr()));
}
