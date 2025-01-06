#include <math.h>
#include <stdio.h>
#include "navmesh.h"
#include "display/draw.h"
#define EPSILON 0.001f

typedef struct {
    float x, y, z;
} Vertex;

Vertex floorVertsA[][3] = {
    {6.091999, 0.000000, -4.032292},
    {-4.204150, 0.000000, -4.032292},
    {6.091999, 0.000000, 4.794376},
    {-4.204150, 0.000000, 4.794376},
    {6.091999, 0.000000, 3.111817},
    {-4.204150, 0.000000, 3.111817},
    {-6.248957, 0.000000, 4.794376},
    {-6.248957, 0.000000, 3.111817},
    {-4.204150, 0.000000, 6.437602},
    {-6.248957, 0.000000, 6.437602},
    {-4.204150, 0.000000, 4.154974},
    {6.091999, 0.000000, 4.154974},
    {-6.248957, 0.000000, 4.154974},
    {-10.350464, 0.000000, 4.794376},
    {-10.350464, 0.000000, 6.437602},
    {6.091999, 0.000000, -0.703134},
    {-4.204150, 0.000000, -0.703134},
    {-5.278532, 0.000000, -4.032292},
    {-5.278532, 0.000000, -0.703134}
};

Vertex floorVertsB[][3] = {
    {81.410355, 0.000000, -1.725043},
    {0.528340, 0.000000, -1.678358},
    {81.404800, 0.000000, 1.678358},
    {0.528340, 0.000000, 1.678358},
    {16.314577, 0.000000, -1.678358},
    {16.314577, 0.000000, 1.678358},
    {20.066959, 0.000000, 1.678358},
    {20.066959, 0.000000, -1.678358},
    {16.314577, 0.000000, -42.024750},
    {20.066959, 0.000000, -42.024750},
    {16.314577, 0.000000, -38.263588},
    {20.066959, 0.000000, -38.263588},
    {81.415901, 0.000000, -42.024750},
    {81.415901, 0.000000, -38.263588},
    {64.470795, 0.000000, -42.024750},
    {64.470795, 0.000000, -38.263588},
    {81.415901, 0.000000, -17.341017},
    {64.470795, 0.000000, -17.341017},
    {74.812531, 0.000000, -42.024750},
    {74.812531, 0.000000, -38.263588},
    {74.812531, 0.000000, -17.341017},
    {65.588554, 0.000000, 1.678358},
    {74.812729, 0.000000, -1.725043},
    {74.812920, 0.000000, 1.678358},
    {65.588394, 0.000000, -1.717177},
    {74.812675, 0.000000, -4.207753},
    {81.411827, 0.000000, -4.207753},
    {65.588394, 0.000000, -4.199866}
};

int floorFaceIndicesA[][4] = {
    {12, 11, 4, 3},
    {16, 17, 6, 5},
    {11, 6, 8, 13},
    {4, 7, 10, 9},
    {4, 11, 13, 7},
    {5, 6, 11, 12},
    {10, 7, 14, 15},
    {1, 2, 17, 16},
    {17, 2, 18, 19}
};

int floorFaceIndicesB[][4] = {
    {5, 2, 4, 6},
    {8, 5, 6, 7},
    {25, 8, 7, 22},
    {11, 12, 10, 9},
    {5, 8, 12, 11},
    {19, 20, 14, 13},
    {10, 12, 16, 15},
    {20, 16, 18, 21},
    {14, 20, 21, 17},
    {15, 16, 20, 19},
    {27, 26, 23, 1},
    {1, 23, 24, 3},
    {23, 25, 22, 24},
    {17, 21, 26, 27},
    {25, 23, 26, 28}
};

int numFacesA = sizeof(floorFaceIndicesA) / sizeof(floorFaceIndicesA[0]);
int numFacesB = sizeof(floorFaceIndicesB) / sizeof(floorFaceIndicesB[0]);

float distanceSquared(float x1, float z1, float x2, float z2) {
    return (x1 - x2) * (x1 - x2) + (z1 - z2) * (z1 - z2);
}

void projectToSegment(Vertex v1, Vertex v2, float *camX, float *camZ) {
    float dx = v2.x - v1.x;
    float dz = v2.z - v1.z;
    float len_squared = dx * dx + dz * dz;

    if (len_squared == 0) return;  // Points are identical

    float t = (( *camX - v1.x) * dx + (*camZ - v1.z) * dz) / len_squared;
    t = fmaxf(0, fminf(1, t)); // Clamp t to [0, 1]
    
    *camX = v1.x + t * dx;
    *camZ = v1.z + t * dz;

    

}

int isPointInQuad(Vertex v0, Vertex v1, Vertex v2, Vertex v3, float camX, float camZ) {

    float minX = fminf(fminf(v0.x, v1.x), fminf(v2.x, v3.x));
    float maxX = fmaxf(fmaxf(v0.x, v1.x), fmaxf(v2.x, v3.x));
    float minZ = fminf(fminf(v0.z, v1.z), fminf(v2.z, v3.z));
    float maxZ = fmaxf(fmaxf(v0.z, v1.z), fmaxf(v2.z, v3.z));

    return (camX >= minX - EPSILON && camX <= maxX + EPSILON &&
            camZ >= minZ - EPSILON && camZ <= maxZ + EPSILON);
}

int check_if_intersects(float *camX, float *camZ, int navmeshSelect) {

    if(navmeshSelect == 1) {
    
        int intersects = 0;

        for (int i = 0; i < numFacesB; i++) {
            Vertex v0 = *floorVertsB[floorFaceIndicesB[i][0] - 1];
            Vertex v1 = *floorVertsB[floorFaceIndicesB[i][1] - 1];
            Vertex v2 = *floorVertsB[floorFaceIndicesB[i][2] - 1];
            Vertex v3 = *floorVertsB[floorFaceIndicesB[i][3] - 1];

            if (isPointInQuad(v0, v1, v2, v3, *camX, *camZ)) {
                intersects = 1;
                break;
            }
        }


        if (!intersects) {
            float minDist = INFINITY;
            Vertex nearestV1, nearestV2;

            for (int i = 0; i < numFacesB; i++) {
                Vertex v0 = *floorVertsB[floorFaceIndicesB[i][0] - 1];
                Vertex v1 = *floorVertsB[floorFaceIndicesB[i][1] - 1];
                Vertex v2 = *floorVertsB[floorFaceIndicesB[i][2] - 1];
                Vertex v3 = *floorVertsB[floorFaceIndicesB[i][3] - 1];

                Vertex edges[4][2] = {{v0, v1}, {v1, v2}, {v2, v3}, {v3, v0}};

                for (int j = 0; j < 4; j++) {
                    float tempCamX = *camX;
                    float tempCamZ = *camZ;
                    projectToSegment(edges[j][0], edges[j][1], &tempCamX, &tempCamZ);
                    


                    float dist = distanceSquared(*camX, *camZ, tempCamX, tempCamZ);



                    if (dist < minDist) {
                        minDist = dist;
                        nearestV1 = edges[j][0];
                        nearestV2 = edges[j][1];
                    }
                }
            }

            projectToSegment(nearestV1, nearestV2, camX, camZ);
        }

        return intersects;
    }
    else {
        int intersects = 0;

        for (int i = 0; i < numFacesA; i++) {
            Vertex v0 = *floorVertsA[floorFaceIndicesA[i][0] - 1];
            Vertex v1 = *floorVertsA[floorFaceIndicesA[i][1] - 1];
            Vertex v2 = *floorVertsA[floorFaceIndicesA[i][2] - 1];
            Vertex v3 = *floorVertsA[floorFaceIndicesA[i][3] - 1];

            if (isPointInQuad(v0, v1, v2, v3, *camX, *camZ)) {
                intersects = 1;
                break;
            }
        }


        if (!intersects) {
            float minDist = INFINITY;
            Vertex nearestV1, nearestV2;

            for (int i = 0; i < numFacesA; i++) {
                Vertex v0 = *floorVertsA[floorFaceIndicesA[i][0] - 1];
                Vertex v1 = *floorVertsA[floorFaceIndicesA[i][1] - 1];
                Vertex v2 = *floorVertsA[floorFaceIndicesA[i][2] - 1];
                Vertex v3 = *floorVertsA[floorFaceIndicesA[i][3] - 1];

                Vertex edges[4][2] = {{v0, v1}, {v1, v2}, {v2, v3}, {v3, v0}};

                for (int j = 0; j < 4; j++) {
                    float tempCamX = *camX;
                    float tempCamZ = *camZ;
                    projectToSegment(edges[j][0], edges[j][1], &tempCamX, &tempCamZ);

                    float dist = distanceSquared(*camX, *camZ, tempCamX, tempCamZ);
                    if (dist < minDist) {
                        minDist = dist;
                        nearestV1 = edges[j][0];
                        nearestV2 = edges[j][1];
                    }
                }
            }

            
            projectToSegment(nearestV1, nearestV2, camX, camZ);
        }

        return intersects;
    }

    
}
