#ifndef SG_SCENE_H
#define SG_SCENE_H

#include "sage_base.h"
#include <inttypes.h>
#include "sg_math.h"
#include "GLFW/glfw3.h"

typedef struct SgCamera {
	v3              position;
	v3              front;
	v3              up;
	float           speed;
	float           fov;
	float           sensitivity;
	float           aspectRatio;
	v2              cursorPosition;
} SgCamera;

typedef struct SgScene {
	SgCamera*       pCamera;
	double          startTime;
	double          currentTime;
	double          previousTime;
	double          deltaTime;
} SgScene;

typedef struct SgTransformUniform {
	m4              model;
	m4              view;
	m4              projection;
} SgTransformUniform;

typedef struct SgCameraTransformInfo {
	v3              moveDirection;
	v2              cursorOffset;
} SgCameraTransformInfo;
void sgSceneInit(SgScene* pScene);
// Really missleading function name atm. This is what you do when you have not enough time to implement the initial design
void sgSceneUpdate(SgScene* pScene);

void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo, SgCamera* pCamera);

void sgInitTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform);
void sgUpdateTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform);

#endif
