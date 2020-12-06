#ifndef SAGE_SCENE_H_
#define SAGE_SCENE_H_ 1

#include "sage_core.h"
#include "sage_math.h"

/* TODO: Make scene API consistent with the rendering API.
 * This sould have been an ECS and concurrency-centered
 * game engine. Having time constrains is awfull...
 * You have to actually get the work done. Even if it is
 * ugly, as long as it works, it works. But having little
 * to no safety is really not my thing. And having no time
 * to implement it in a somewhat personal project drives me
 * insane. Like seriously. The rendering part will give me
 * tons and tons of headache once I start fixing the API
 * and the data duplication things that I made when making
 * the descriptor sets that are different between drawcalls
 * but are binded to the same binding slot and have the same
 * layout... Oh, and I also have to make the cleanup function.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


typedef struct SgCamera {
	v3              position;
	v3              front;
	v3              up;
	float           speed;
	float           fov;         // Radians
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

void sgSceneInit(SgScene* pScene);
void sgSceneUpdate(SgScene* pScene);

void sgInitTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform);
void sgUpdateTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform);

#endif
