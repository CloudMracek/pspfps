#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <pspmoduleinfo.h>
#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>


#include "./system/callbacks.h"
#include "controls.h"
#include "display/draw.h"
#include "audio.h"
#include "bwm.h"
#include "pspthreadman.h"
#include "psptypes.h"
#include "texture.h"
#include "navmesh.h"
#include "camera.h"
#include "filesystem.h"
#include "stage2.h"
#include "psprtc.h"

PSP_MODULE_INFO("PSPFPS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int running = 1;

BWM_Header* floorHeader;
BWM_Vertex* floorVertices;
BWM_VertexIndice* floorIndices;


Texture* anim[46];

int main(int argc, char* argv[]) {

    int i = 0;
    int nextStage = 0;
    int animframe = 0;
    int portalSpawned = 0;
    SceCtrlData pad;
    int messageMp3Instance;

    
    int state = 0;
    double stateTimer = 0;
    
    setup_callbacks();
    init_display();

    start_frame();
    draw_string("LOADING", 210,120,0xffffffff, 0);
    end_frame();

    unsigned char* __attribute__((aligned(16))) roomData;
    roomData = malloc(getFileSize("assets/models/room.bwm"));

    FILE* room = fopen("assets/models/room.bwm", "rb");
    fread(roomData, sizeof(char) ,getFileSize("assets/models/room.bwm"), room);
    fclose(room);


    unsigned char* __attribute__((aligned(16))) horizonData;
    horizonData = malloc(getFileSize("assets/models/horizon.bwm"));

    FILE* horizon = fopen("assets/models/horizon.bwm", "rb");
    fread(horizonData, sizeof(char) ,getFileSize("assets/models/horizon.bwm"), horizon);
    fclose(horizon);

    unsigned char* __attribute__((aligned(16))) recorderData;
    recorderData = malloc(getFileSize("assets/models/recorder.bwm"));

    FILE* recorder = fopen("assets/models/recorder.bwm", "rb");
    fread(recorderData, sizeof(char) ,getFileSize("assets/models/recorder.bwm"), recorder);
    fclose(recorder);

    BWM_Header* roomHeader = bwm_header(roomData);
    BWM_Vertex* roomVertices = bwm_vertices(roomData);
    BWM_VertexIndice* roomIndices = bwm_vertex_indices(roomData);

    BWM_Header* horizonHeader = bwm_header(horizonData);
    BWM_Vertex* horizonVertices = bwm_vertices(horizonData);
    BWM_VertexIndice* horizonIndices = bwm_vertex_indices(horizonData);

    BWM_Header* recorderHeader = bwm_header(recorderData);
    BWM_Vertex* recorderVertices = bwm_vertices(recorderData);
    BWM_VertexIndice* recorderIndices = bwm_vertex_indices(recorderData);

    for(int i = 0; i < 46; i+=1) {
        char* filename = malloc(100);
        sprintf(filename, "assets/textures/horizon_new_%02d_00_00_resized.png", i);
        anim[i] = load_texture(filename, GU_FALSE);
        free(filename);
    }

    //Texture* dumbass = load_texture("assets/textures/dumbass.png", GU_TRUE);
    Texture* roomTex = load_texture("assets/textures/room.png", GU_FALSE);
    Texture* recorderTex = load_texture("assets/textures/recorder.png", GU_FALSE);


    init_controls();
    mp3_init();

    uint64_t tick_resolution = sceRtcGetTickResolution();
    uint64_t last_tick;

    while(!should_quit() && running) {

        sceRtcGetCurrentTick(&last_tick);
        start_frame();

        sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
        sceGuEnable(GU_BLEND);

        sceGuClearColor(0x3e3e3e3e);
        sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

        sceGuAmbient(0xff111111);
        
        ScePspFVector3 pos2 = {0, 1, 0};
        sceGuLight(1,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos2);
        sceGuLightColor(1,GU_DIFFUSE,0xffffffff);
        sceGuLightColor(1,GU_SPECULAR,0xffffffff);
        sceGuLightAtt(1,1.0f,0.05f,0.05f);

        	
        sceGumMatrixMode(GU_PROJECTION);
		sceGumLoadIdentity();
		sceGumPerspective(75.0f,16.0/9.0f,0.01f,1000.0f);    


        /*bind_texture(dumbass);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { 1, 0, -3 };
			ScePspFVector3 rot = { i * 0.79f * (GU_PI/180.0f), i * 0.98f * (GU_PI/180.0f), 0};
			sceGumTranslate(&pos);
			sceGumRotateXYZ(&rot);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, header->faces_num*3, indices, vertices);        
        */

        bind_texture(roomTex);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { 0, -4, 0 };
			sceGumTranslate(&pos);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, roomHeader->faces_num*3, roomIndices, roomVertices);

        bind_texture(recorderTex);
        sceGumMatrixMode(GU_MODEL);
		sceGumLoadIdentity();
		{
			ScePspFVector3 pos = { -8, -1.1, 0 };
			sceGumTranslate(&pos);
		}

        sceGumDrawArray(GU_TRIANGLES, GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_NORMAL_32BITF | GU_VERTEX_32BITF | GU_TRANSFORM_3D, recorderHeader->faces_num*3, recorderIndices, recorderVertices);

        if(portalSpawned) {
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

            
        ScePspFVector3 camPos = getCameraPosition();
        int dist = distance3D(-camPos.x, -camPos.y, -camPos.z, 8, -1.1f, 0);

        if(dist < 3 && state == 0) {
            draw_string("Press the right bumper to play", 50,200,0xffffffff, 0);
            if( get_button_down(PSP_CTRL_RTRIGGER)) {
                messageMp3Instance = start_mp3_playback("assets/sounds/Test.mp3", 0);
                state = 1;
                stateTimer = 0;
            }
        }

        char buff[100];
        sprintf(buff, "%d,%lf", state, stateTimer);
        draw_string(buff, 0,64,0xffffffff, 0);

        if(stateTimer > 15 && state == 2) {
            
            messageMp3Instance = start_mp3_playback("assets/sounds/portal.mp3", -1);
            sceGuDisable(GU_LIGHT1);
            ScePspFVector3 pos = { 13, -1.1, -5 };
            sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
            sceGuLightColor(0,GU_DIFFUSE,0xffff1111);
            sceGuLightColor(0,GU_SPECULAR,0xffff1111);
            sceGuLightAtt(0,0.3f,0.1f,0.1f);
            portalSpawned = 1;
            state = 3;
            stateTimer = 0;
        }
        


        end_frame();
    
        if(state == 3) {
            int dist = distance3D(camPos.x, camPos.y, camPos.z, 13, 0, -5);
            if( dist < 3) {
                nextStage = 1;
            }
        }

        if(state == 1 && is_mp3_playback_finished(messageMp3Instance) == 1) {
            state = 2;
            stateTimer = 0;
        }


        mp3_update();
        

        if(1) {
            stop_mp3_playback(messageMp3Instance);
            start_frame();
            sceGuClearColor(0xff000000);
            sceGuClearDepth(0);
		    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);
            typedef struct {
                float s, t;       // Texture coordinates
                unsigned int c;   // Color
                float x, y, z;    // Position
            } VERT;

            VERT* v = sceGuGetMemory(sizeof(VERT) * 4); // 4 vertices for the quad

            // Bottom-left vertex
            v[0].s = 0.0f; v[0].t = 0.0f; v[0].c = 0xFFFFFFFF; // White color
            v[0].x = 0.0f; v[0].y = 0.0f; v[0].z = 0.0f;

            // Bottom-right vertex
            v[1].s = 128.0f; v[1].t = 0.0f; v[1].c = 0xFFFFFFFF; // White color
            v[1].x = (float)480; v[1].y = 0.0f; v[1].z = 0.0f;

            // Top-left vertex
            v[2].s = 0.0f; v[2].t = 127.0f; v[2].c = 0xFFFFFFFF; // White color
            v[2].x = 0.0f; v[2].y = (float)272; v[2].z = 0.0f;

            // Top-right vertex
            v[3].s = 128.0f; v[3].t = 127.0f; v[3].c = 0xFFFFFFFF; // White color
            v[3].x = (float)480; v[3].y = (float)272; v[3].z = 0.0f;

            bind_texture(anim[1]);
            sceGumDrawArray(GU_TRIANGLE_STRIP, 
                            GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D,
                            4, 0, v);
            draw_string("LOADING", 210,120,0xff00ffff, 0);
            end_frame();
            

            free(roomData);
            free(horizonData);
            free(horizonData);

            free_texure(recorderTex);
            free_texure(roomTex);
            
            for(int i = 0; i < 46; i++) {
                free_texure(anim[i]);
            }

            stage2();
            running = 0;
            continue;
        }
        uint64_t current_tick;
        sceRtcGetCurrentTick(&current_tick);

        double dt = (double)(current_tick - last_tick) / ((double)tick_resolution);

        last_tick = current_tick;


        stateTimer += dt;

        animframe += 1;
        if(animframe == 46) {
            animframe = 0;
        }


        update_controls();
        update_camera(0, dt);

    }


    free(roomData);
    free(horizonData);
    free(horizonData);

    free_texure(recorderTex);
    free_texure(roomTex);
    
    for(int i = 0; i < 46; i++) {
        free_texure(anim[i]);
    }


    term_graphics();


    return 0;
}