#pragma once
#include "psptypes.h"

void resetCamera();

void setSteps(int type);

void destroySteps();

void setLimits(float limitX, float limitY, float limitZ);

void update_camera(int navmeshSelect, double dt);

ScePspFVector3 getCameraPosition();

ScePspFVector3 getCameraRotation();
