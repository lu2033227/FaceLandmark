#ifndef POSE_H
#define POSE_H

typedef struct
{
	float x;
	float y;
	int id;
}egp_NodePtrArr;

#include "AIMG.h"

void PoseEstimation(egp_NodePtrArr* nodes, int pt_num, float* roll, float* yaw, float* pitch);

#endif
