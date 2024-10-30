#include <pspmoduleinfo.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <stdio.h>
#include <stdlib.h>


#include "./system/callbacks.h"
#include "controls.h"
#include "display/draw.h"
#include "navmesh.h"
#include "camera.h"
#include "filesystem.h"
#include "bwm.h"
#include "psprtc.h"
#include "psptypes.h"
#include "texture.h"
#include "stage2.h"


void stage2() {

    int animframe = 0, animframe2 = 0;
    SceCtrlData pad;
    Texture* anim[46];

    resetCamera();

    unsigned char* __attribute__((aligned(16))) dungeonData;
    dungeonData = malloc(getFileSize("assets/models/dungeon.bwm"));

    FILE* dungeon = fopen("assets/models/dungeon.bwm", "rb");
    fread(dungeonData, sizeof(char) ,getFileSize("assets/models/dungeon.bwm"), dungeon);
    fclose(dungeon);

    BWM_Header* dungeonHeader = bwm_header(dungeonData);
    BWM_Vertex* dungeonVerts = bwm_vertices(dungeonData);
    BWM_VertexIndice* dungeonIndices = bwm_vertex_indices(dungeonData);

    unsigned char* __attribute__((aligned(16))) strutsData;
    strutsData = malloc(getFileSize("assets/models/struts.bwm"));
    FILE* struts = fopen("assets/models/struts.bwm", "rb");
    fread(strutsData, sizeof(char) ,getFileSize("assets/models/struts.bwm"), struts);
    fclose(struts);

    BWM_Header* strutsHeader = bwm_header(strutsData);
    BWM_Vertex* strutsVerts = bwm_vertices(strutsData);
    BWM_VertexIndice* strutsIndices = bwm_vertex_indices(strutsData);

    unsigned char* __attribute__((aligned(16))) horizonData;
    horizonData = malloc(getFileSize("assets/models/horizon2.bwm"));

    FILE* horizon = fopen("assets/models/horizon2.bwm", "rb");
    fread(horizonData, sizeof(char) ,getFileSize("assets/models/horizon2.bwm"), horizon);
    fclose(horizon);

    BWM_Header* horizonHeader = bwm_header(horizonData);
    BWM_Vertex* horizonVertices = bwm_vertices(horizonData);
    BWM_VertexIndice* horizonIndices = bwm_vertex_indices(horizonData);

    unsigned char* __attribute__((aligned(16))) flashlightData;
    flashlightData = malloc(getFileSize("assets/models/flashlight.bwm"));

    FILE* flashlight = fopen("assets/models/flashlight.bwm", "rb");
    fread(flashlightData, sizeof(char) ,getFileSize("assets/models/flashlight.bwm"), flashlight);
    fclose(flashlight);

    BWM_Header* flashlightHeader = bwm_header(flashlightData);
    BWM_Vertex* flashlightVerts = bwm_vertices(flashlightData);
    BWM_VertexIndice* flashlightIndices = bwm_vertex_indices(flashlightData);

    unsigned char* __attribute__((aligned(16))) fishData;
    fishData = malloc(getFileSize("assets/models/fish.bwm"));

    FILE* fish = fopen("assets/models/fish.bwm", "rb");
    fread(fishData, sizeof(char) ,getFileSize("assets/models/fish.bwm"), flashlight);
    fclose(fish);

    BWM_Header* fishHeader = bwm_header(fishData);
    BWM_Vertex* fishVerts = bwm_vertices(fishData);
    BWM_VertexIndice* fishIndices = bwm_vertex_indices(fishData);

    Texture* dungeonTex = load_texture("assets/textures/dungeon.png", GU_FALSE);
    Texture* strutsTex = load_texture("assets/textures/struts.png", GU_FALSE);
    Texture* flashlightTex = load_texture("assets/textures/flashlight.png", GU_FALSE);
    Texture* fishTex = load_texture("assets/textures/fish.png", GU_FALSE);

    for(int i = 0; i < 46; i+=1) {
        char* filename = malloc(100);
        sprintf(filename, "assets/textures/horizon_new_%02d_00_00_resized.png", i);
        anim[i] = load_texture(filename, GU_FALSE);
        free(filename);
    }

    sceGuEnable(GU_LIGHTING);
    uint64_t tick_resolution = sceRtcGetTickResolution();
    uint64_t last_tick;

    while(1) {
        sceRtcGetCurrentTick(&last_tick);

        start_frame();

        ScePspFVector3 camPos = getCameraPosition();
        ScePspFVector3 camRot = getCameraRotation();

        ScePspFVector3 rot1 = {0, camRot.y, 0};
        ScePspFVector3 rot2 = {camRot.x, 0, 0};

        ScePspFMatrix4 lightMatrix;
        gumLoadIdentity(&lightMatrix);
        gumTranslate(&lightMatrix,&camPos);
        gumRotateXYZ(&lightMatrix,&rot1);
        gumRotateXYZ(&lightMatrix,&rot2);
    
        ScePspFVector3 lightPos = { lightMatrix.w.x, lightMatrix.w.y, lightMatrix.w.z };
        ScePspFVector3 lightDir = { lightMatrix.z.x, lightMatrix.z.y, lightMatrix.z.z }; 

        sceGuLight(0,GU_SPOTLIGHT,GU_DIFFUSE,&lightPos);
        sceGuLightSpot(0,&lightDir, 5.0, 0.3);
        sceGuLightColor(0,GU_DIFFUSE,0x00ffffff);
        sceGuLightAtt(0, 1.0f, 0.000f, 0.000f); // Example adjustments
        sceGuAmbient(0x00202020);

        if(animframe2 < 512) {
            sceGuDisable(GU_LIGHT0);
        }
        else {
            sceGuEnable(GU_LIGHT0);
        }


        ScePspFVector3 pos = { 0, 0, 0 };
        sceGuLight(1,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
        sceGuLightColor(1,GU_DIFFUSE,0xffff1111);
        sceGuLightColor(1,GU_SPECULAR,0xffff1111);
        sceGuLightAtt(1,1.0f,0.1f,0.1f);

        if(animframe2 < 256) {
            sceGuEnable(GU_LIGHT1);
        }
        else {
            sceGuDisable(GU_LIGHT1);
        }
        
        sceGuAmbient(0xff000000);

        sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        sceGuEnable(GU_BLEND);

        sceGuClearColor(0xff000000);
        sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

        sceGuAmbient(0xff000000);

        	
        sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(75.0f,16.0/9.0f,0.5f,1000.0f);    




        bind_texture(dungeonTex);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { 0, -4, 0 };
			sceGumTranslate(&pos);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, dungeonHeader->faces_num*3, dungeonIndices, dungeonVerts);


        bind_texture(strutsTex);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { 0, -4, 0 };
			sceGumTranslate(&pos);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, strutsHeader->faces_num*3, strutsIndices, strutsVerts);

        bind_texture(fishTex);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { -18, 0, 40 };
            ScePspFVector3 rot = {0, -45.0f / 180.0f * GU_PI, 0};
			sceGumTranslate(&pos);
            sceGumRotateXYZ(&rot);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, fishHeader->faces_num*3, fishIndices, fishVerts);

        if(animframe2 < 256) {        
            bind_texture(anim[animframe]);
            sceGumMatrixMode(GU_MODEL);
            sceGumLoadIdentity();
            {
                ScePspFVector3 pos = { 0, -4, 0 };
                sceGumTranslate(&pos);
            }
            sceGuDisable(GU_LIGHTING);
            sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, horizonHeader->faces_num*3, horizonIndices, horizonVertices);
            sceGuEnable(GU_LIGHTING);
        }


        bind_texture(flashlightTex);
        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();
        {
            ScePspFVector3 pos = { 3, -2, -3 };
            ScePspFVector3 rot = {0, 0, 180.0f / 180.0f * GU_PI};
            sceGumTranslate(&pos);
            sceGumRotateXYZ(&rot);
        }

        sceGuDisable(GU_LIGHT0);
        sceGuDisable(GU_LIGHT1);
        sceGuDisable(GU_DEPTH_TEST);
        sceGuAmbient(0xff333333);
        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, flashlightHeader->faces_num*3, flashlightIndices, flashlightVerts);
        sceGuEnable(GU_DEPTH_TEST);
        sceGuEnable(GU_LIGHT0);
        sceGuEnable(GU_LIGHT1);




        end_frame();



        animframe++;
        if(animframe == 46) {
            animframe = 0;
        }

        animframe2++;

        uint64_t current_tick;
        sceRtcGetCurrentTick(&current_tick);

        double dt = (double)(current_tick - last_tick) / ((double)tick_resolution);

        last_tick = current_tick;
        update_controls();
        update_camera(1, dt);
    }
    free(dungeonData);
}