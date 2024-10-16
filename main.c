#include "pspmoduleinfo.h"
#include <math.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>

#include "audio.h"
#include "display/draw.h"
#include "inc/bwm.h"
#include "psptypes.h"
#include "system/callbacks.h"

#include "texture.h"
#include <stdio.h>
#include <stdlib.h>

PSP_MODULE_INFO("PSPFPS", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int running = 1;

struct Vertex {
  float u, v;
  unsigned int color;
  float x, y, z;
};

float camX, camY, camZ;
float rotX, rotY, rotZ;

ScePspFVector3 lightPos = {0, 0, 0};

void apply_camera() {
  sceGumMatrixMode(GU_VIEW);
  sceGumLoadIdentity();

  ScePspFVector3 r = {rotX / 180.0f * 3.14159f, rotY / 180.0f * 3.14159f,
                      rotZ / 180.0f * 3.14159f};
  sceGumRotateXYZ(&r);

  ScePspFVector3 v = {camX, camY, camZ};
  sceGumTranslate(&v);

  sceGumMatrixMode(GU_MODEL);
  sceGumLoadIdentity();
}

void update_camera() {
  SceCtrlData pad;
  sceCtrlReadBufferPositive(&pad, 1);

  // Calculate movement based on current rotation
  float moveSpeed = 0.5f;
  float moveX = moveSpeed * sin(-rotY * (GU_PI / 180.0f));
  float moveZ = moveSpeed * cos(-rotY * (GU_PI / 180.0f));

  // Adjust camera position based on movement buttons (WSAD style)
  if (pad.Buttons & PSP_CTRL_CROSS) {
    camX -= moveX;
    camZ -= moveZ; // Flip the direction for back movement
  }
  if (pad.Buttons & PSP_CTRL_TRIANGLE) {
    camX += moveX;
    camZ += moveZ;
  }
  if (pad.Buttons & PSP_CTRL_SQUARE) {
    camX += moveZ;
    camZ -= moveX; // Flip the direction for left movement
  }
  if (pad.Buttons & PSP_CTRL_CIRCLE) {
    camX -= moveZ; // Flip the direction for right movement
    camZ += moveX;
  }
  if (pad.Buttons & PSP_CTRL_UP) {
    rotX--;
  }
  if (pad.Buttons & PSP_CTRL_DOWN) {
    rotX++;
  }
  if (pad.Buttons & PSP_CTRL_LEFT) {
    rotY--;
  }
  if (pad.Buttons & PSP_CTRL_RIGHT) {
    rotY++;
  }

  // Correct the rotation signs for sine and cosine
  moveX = moveSpeed * sin(rotY * (GU_PI / 180.0f));
  moveZ = moveSpeed * cos(rotY * (GU_PI / 180.0f));
}

void bind_texture(Texture *tex) {
  if (tex == NULL)
    return;

  sceGuTexMode(GU_PSM_8888, 0, 0, 1);
  sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
  sceGuTexFilter(GU_NEAREST, GU_NEAREST);
  sceGuTexWrap(GU_REPEAT, GU_REPEAT);
  sceGuTexImage(0, tex->pW, tex->pH, tex->pW, tex->data);
}

long getFileSize(const char *filename) {
  FILE *file = fopen(filename, "rb"); // Open the file in binary mode

  if (file == NULL) {
    perror("Error opening file");
    return -1; // Error opening file
  }

  fseek(file, 0, SEEK_END); // Move the file pointer to the end of the file
  long size = ftell(
      file);    // Get the position of the file pointer, which is the file size
  fclose(file); // Close the file

  return size;
}

int main(int argc, char *argv[]) {
  setup_callbacks();
  init_display();

  unsigned char *__attribute__((aligned(16))) cubeData;
  cubeData = malloc(getFileSize("cube.bwm"));

  FILE *cube = fopen("cube.bwm", "rb");
  fread(cubeData, sizeof(char), getFileSize("cube.bwm"), cube);
  fclose(cube);

  unsigned char *__attribute__((aligned(16))) floorData;
  floorData = malloc(getFileSize("floor.bwm"));

  FILE *floor = fopen("floor.bwm", "rb");
  fread(floorData, sizeof(char), getFileSize("floor.bwm"), floor);
  fclose(floor);

  BWM_Header *header = bwm_header(cubeData);
  BWM_Vertex *vertices = bwm_vertices(cubeData);
  BWM_VertexIndice *indices = bwm_vertex_indices(cubeData);

  BWM_Header *floorHeader = bwm_header(floorData);
  BWM_Vertex *floorVertices = bwm_vertices(floorData);
  BWM_VertexIndice *floorIndices = bwm_vertex_indices(floorData);

  Texture *dumbass = load_texture("dumbass.png", GU_TRUE);
  Texture *floorTex = load_texture("grid.png", GU_TRUE);
  int i = 0;

  SceCtrlData pad;

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  pspDebugScreenInit();
  pspDebugScreenClear();
  mp3_init();

  mp3_load("chipi.mp3");

  int thid = sceKernelCreateThread("sound", mp3_update, 0x11, 0xFFFF, 0, 0);
  sceKernelStartThread(thid, 0, 0);

  while (!should_quit() && running) {

    start_frame();

    sceGuClearColor(0x3e3e3e3e);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

    // Update camera first
    update_camera();
    apply_camera();

    // Calculate light position based on the object’s transformations
    ScePspFVector3 pos = {0, 0, -4.5f};
    ScePspFVector3 rot = {i * 0.79f * (GU_PI / 180.0f),
                          i * 0.98f * (GU_PI / 180.0f), 0};

    // Setup the light position in world space
    ScePspFVector3 lightPos = {0, 2, -4.5f};
    sceGuLight(0, GU_POINTLIGHT, GU_DIFFUSE_AND_SPECULAR, &lightPos);
    sceGuLightColor(0, GU_DIFFUSE, 0xffffffff);
    sceGuLightColor(0, GU_SPECULAR, 0xffffffff);
    sceGuLightAtt(0, 0.0f, 1.0f, 0.0f);
    sceGuSpecular(12.0f);
    sceGuAmbient(0x00);

    // Draw the rotating object
    bind_texture(dumbass);
    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    // Apply transformations for the object
    sceGumTranslate(&pos);
    sceGumRotateXYZ(&rot);

    sceGumDrawArray(GU_TRIANGLES,
                    GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 |
                        GU_VERTEX_32BITF | GU_TRANSFORM_3D,
                    header->faces_num * 3, indices, vertices);

    // Draw the floor
    bind_texture(floorTex);
    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    ScePspFVector3 floorPos = {0, -4.0f, 0};
    sceGumTranslate(&floorPos);

    sceGumDrawArray(GU_TRIANGLES,
                    GU_INDEX_16BIT | GU_TEXTURE_32BITF | GU_COLOR_8888 |
                        GU_VERTEX_32BITF | GU_TRANSFORM_3D,
                    floorHeader->faces_num * 3, floorIndices, floorVertices);

    end_frame();
    i++;
  }
  term_graphics();

  return 0;
}