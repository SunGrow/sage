#ifndef INPUT_FUNCS
#define INPUT_FUNCS 1

#include "sage.h"
#include "sage_rend.h"
#include "sage_scene.h"
#include "sage_input.h"
#include "log.h"
#include "math.h"

typedef struct SgCameraTransformInfo {
	v3              moveDirection;
	v2              cursorOffset;
	double          deltaTime;
	SgCamera        camera;
} SgCameraTransformInfo;

void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo, SgCamera* pCamera);

void keyPressForward(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyPressUp(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyPressLeft(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyPressRight(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyPressDown(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyPressBack(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);
void keyShoot(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);

void rotateCamera(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow);

#endif
