#ifndef SG_CAMERA_H
#define SG_CAMERA_H

#include <inttypes.h>

#include "GLFW/glfw3.h"
#include "sg_base.h"
#include "sg_math.h"

typedef struct SgTmpCamera {
	v3 position;
	v3 front;
	v3 up;
	float speed;
	float fov;
	float sensitivity;
	float aspectRatio;
	v2 cursorPosition;
} SgTmpCamera;

typedef struct SgTimedCamera {
	SgTmpCamera* pCamera;
	double startTime;
	double currentTime;
	double previousTime;
	double deltaTime;
} SgTimedCamera;

typedef struct SgTransformUniform {
	m4 model;
	m4 view;
	m4 projection;
} SgTransformUniform;

typedef struct SgCameraTransformInfo {
	v3 moveDirectionPos;
	v2 cursorOffset;
} SgCameraTransformInfo;
void sgTimedCameraInit(SgTimedCamera* pTimedCamera);
// Really missleading function name atm. This is what you do when you have not
// enough time to implement the initial design
void sgTimedCameraUpdate(SgTimedCamera* pTimedCamera);

void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo,
                       SgTmpCamera* pCamera);

void sgInitTransformUniform(const SgTmpCamera* pCamera,
                            SgTransformUniform* pTransformUniform);
void sgUpdateTransformUniform(const SgTmpCamera* pCamera,
                              SgTransformUniform* pTransformUniform);

#endif
