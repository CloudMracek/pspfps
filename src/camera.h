#pragma once
#include "psptypes.h"

void resetCamera();

void update_camera(int navmeshSelect, double dt);

ScePspFVector3 getCameraPosition();

ScePspFVector3 getCameraRotation();
