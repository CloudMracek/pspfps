#include "camera.h"

#include <math.h>
#include <pspgum.h>
#include <pspgu.h>
#include <pspctrl.h>
#include <stdlib.h>

#include "audio.h"
#include "controls.h"
#include "navmesh.h"

float camX = 0, camY = 0, camZ = 0;
float rotX = 0, rotY = 0, rotZ = 0;

float camXLimit = INFINITY;
float camYLimit = INFINITY;
float camZLimit = INFINITY;

int stepsSound = -1;

void resetCamera() {
    camX = 0;
    camY = 0;
    camZ = 0;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
}

void setSteps(int type) {
    if(stepsSound != -1) {
        stop_mp3_playback(stepsSound);
    }
    if(type == 1) {
        stepsSound = start_mp3_playback("assets/sounds/stepsecho.mp3", -1);
    }
    else {
        stepsSound = start_mp3_playback("assets/sounds/steps.mp3", -1);
    }
   
}

void destroySteps() {
    stop_mp3_playback(stepsSound);
    stepsSound = -1;
}

void setLimits(float limitX, float limitY, float limitZ) {
    camXLimit = limitX;
    camYLimit = limitY;
    camZLimit = limitZ;
}

float wrapAngle(float angle) {
    angle = fmod(angle, 360.0f); // Get the remainder when divided by 360

    // Adjust if the angle is negative
    if (angle < 0) {
        angle += 360.0f;
    }

    return angle; // Return the wrapped angle
}

void apply_camera(int navmeshSelect) {

    // First, calculate radians for rotation as in the original
    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    ScePspFVector3 r = {rotX / 180.0f * 3.14159f, rotY / 180.0f * 3.14159f, rotZ / 180.0f * 3.14159f};
    sceGumRotateXYZ(&r);

    // Assume the camera position is updated to ensure it's within the walkable area
    check_if_intersects(&camX, &camZ, navmeshSelect);

    ScePspFVector3 v = {camX, camY, camZ};

    sceGumTranslate(&v);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();
}



void update_camera(int navmeshSelect, double dt) {
    // Calculate movement based on current rotation
    float moveSpeed = 5.0f;
    float moveX = moveSpeed * sin(-rotY * (GU_PI / 180.0f)) * dt;
    float moveZ = moveSpeed * cos(-rotY * (GU_PI / 180.0f)) * dt;

    uint8_t doSound = 0;

    // Adjust camera position based on movement buttons (WSAD style)
    if (get_button(PSP_CTRL_CROSS)) {
        camX -= moveX;
        camZ -= moveZ;  // Flip the direction for back movement
        doSound = 1;
    }
    if (get_button(PSP_CTRL_TRIANGLE)) {
        camX += moveX;
        camZ += moveZ;
        doSound = 1;
    }
    if (get_button(PSP_CTRL_SQUARE)) {
        camX += moveZ;
        camZ -= moveX;  // Flip the direction for left movement
        doSound = 1;
    }
    if (get_button(PSP_CTRL_CIRCLE)) {
        camX -= moveZ;  // Flip the direction for right movement
        camZ += moveX;
        doSound = 1;
    }

    if(doSound) {
        play(stepsSound);
    }
    else {
        pause(stepsSound);
    }

    // Apply limits to camX, camY, and camZ
    if (camX > camXLimit) camX = camXLimit;
    if (camX < -camXLimit) camX = -camXLimit;

    if (camY > camYLimit) camY = camYLimit;
    if (camY < -camYLimit) camY = -camYLimit;

    if (camZ > camZLimit) camZ = camZLimit;
    if (camZ < -camZLimit) camZ = -camZLimit;

    // Rotation deadzone and sensitivity
    int deadzone = 50;
    float sensitivity = 1.0f;

    int adjusted_Lx = (abs(get_Lx() - 128) < deadzone) ? 128 : (128 + (int)((get_Lx() - 128) * sensitivity * dt));
    int adjusted_Ly = (abs(get_Ly() - 128) < deadzone) ? 128 : (128 + (int)((get_Ly()- 128) * sensitivity * dt));

    rotX += adjusted_Ly - 128;
    rotY += adjusted_Lx - 128;

    if (rotX > 90) rotX = 90;
    if (rotX < -90) rotX = -90;

    rotY = wrapAngle(rotY);

    // Recalculate movement directions based on corrected rotation angles
    moveX = moveSpeed * sin(rotY * (GU_PI / 180.0f));
    moveZ = moveSpeed * cos(rotY * (GU_PI / 180.0f));

    // Apply the camera position and orientation
    apply_camera(navmeshSelect);
}


ScePspFVector3 getCameraPosition() {
    ScePspFVector3 camPos = { .x = -camX, .y = -camY, .z = -camZ };
    return camPos;
}

// Function to get camera rotation
ScePspFVector3 getCameraRotation() {
    // Ensure rotY is within 0-360
    rotY = wrapAngle(rotY);
    return (ScePspFVector3){-rotX * (GU_PI / 180.0f), -rotY * (GU_PI / 180.0f), -rotZ * (GU_PI / 180.0f)};
}