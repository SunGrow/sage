#ifndef SAGE_CAMERA_H_
#define SAGE_CAMERA_H_ 1
#include "sage_math.h"

typedef struct SgTmpCamera {
	v3    position;
	v3    front;
	v3    up;
	float speed;
	float fov;  // Radians
	float sensitivity;
	float aspectRatio;
	v2    cursorPosition;
} SgTmpCamera;

typedef struct SgTimedCamera {
	SgTmpCamera* pCamera;
	double       startTime;
	double       currentTime;
	double       previousTime;
	double       deltaTime;
} SgTimedCamera;

typedef struct SgTransformUniform {
	m4 model;
	m4 view;
	m4 projection;
} SgTransformUniform;

void sgTimedCameraInit(SgTimedCamera* pTimedCamera);
void sgTimedCameraUpdate(SgTimedCamera* pTimedCamera);

void sgInitTransformUniform(const SgTmpCamera*  pCamera,
                            SgTransformUniform* pTransformUniform);
void sgUpdateTransformUniform(const SgTmpCamera*  pCamera,
                              SgTransformUniform* pTransformUniform);

#endif
