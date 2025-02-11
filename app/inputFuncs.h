#ifndef INPUT_FUNCS
#define INPUT_FUNCS 1

#include "log.h"
#include "math.h"
#include "sage.h"
#include "sage_input.h"
#include "sage_rend.h"
#include "sage_scene.h"

typedef struct SgCameraTransformInfo {
	v3          moveDirectionPos;
	v3          moveDirectionNeg;
	v2          cursorOffset;
	double      deltaTime;
	SgTmpCamera camera;
} SgCameraTransformInfo;

void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo,
                       SgTmpCamera*                 pCamera);

void keyPressForward(SgBool   isPressed,
                     double   rangeX,
                     double   rangeY,
                     SgActor  actor,
                     SgWindow pWindow);
void keyPressUp(SgBool   isPressed,
                double   rangeX,
                double   rangeY,
                SgActor  actor,
                SgWindow pWindow);
void keyPressLeft(SgBool   isPressed,
                  double   rangeX,
                  double   rangeY,
                  SgActor  actor,
                  SgWindow pWindow);
void keyPressRight(SgBool   isPressed,
                   double   rangeX,
                   double   rangeY,
                   SgActor  actor,
                   SgWindow pWindow);
void keyPressDown(SgBool   isPressed,
                  double   rangeX,
                  double   rangeY,
                  SgActor  actor,
                  SgWindow pWindow);
void keyPressBack(SgBool   isPressed,
                  double   rangeX,
                  double   rangeY,
                  SgActor  actor,
                  SgWindow pWindow);
void keyShoot(SgBool   isPressed,
              double   rangeX,
              double   rangeY,
              SgActor  actor,
              SgWindow pWindow);

void rotateCamera(SgBool   isPressed,
                  double   rangeX,
                  double   rangeY,
                  SgActor  actor,
                  SgWindow pWindow);

void lightKeyPressForward(SgBool   isPressed,
                          double   rangeX,
                          double   rangeY,
                          SgActor  actor,
                          SgWindow pWindow);
void lightKeyPressUp(SgBool   isPressed,
                     double   rangeX,
                     double   rangeY,
                     SgActor  actor,
                     SgWindow pWindow);
void lightKeyPressLeft(SgBool   isPressed,
                       double   rangeX,
                       double   rangeY,
                       SgActor  actor,
                       SgWindow pWindow);
void lightKeyPressRight(SgBool   isPressed,
                        double   rangeX,
                        double   rangeY,
                        SgActor  actor,
                        SgWindow pWindow);
void lightKeyPressDown(SgBool   isPressed,
                       double   rangeX,
                       double   rangeY,
                       SgActor  actor,
                       SgWindow pWindow);
void lightKeyPressBack(SgBool   isPressed,
                       double   rangeX,
                       double   rangeY,
                       SgActor  actor,
                       SgWindow pWindow);
#endif
