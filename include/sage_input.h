#ifndef SAGE_INPUT_H
#define SAGE_INPUT_H

#include "sage_core.h"
#include "sage_rend.h"

SG_DEFINE_HANDLE(SgActiveContexts);
typedef void* SgActor;
typedef void (*SgInputAction) (/* State */ uint32_t, /* rangeX */ double, /* rangeY */ double, SgActor, SgWindow);

typedef enum SgInputType {
	SG_INPUT_TYPE_KEYBOARD = BIT(0),
	SG_INPUT_TYPE_MOUSE    = BIT(1),
} SgInputType;

typedef struct SgInputSignal {
	uint32_t          key;
	uint32_t          mod;

	char*             keyName;
	char*             modName;

	SgInputType       type;
} SgInputSignal;

typedef struct SgActiveContextsCreateInfo {
	SgActor*          pActors;
	char**            pActorNames;
	uint32_t          actorCount;

	SgInputSignal*    pInputSignals;
	uint32_t          signalCount;

	SgInputAction*    pActionFuncs;
	char**            pActionNames;
	uint32_t          actionCount;

	SgFile            file;
} SgActiveContextsCreateInfo;

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts* pContexts);

void sgSetActiveContexts(SgActiveContexts activeContexts, SgApp *pApp);

void sgUnloadContexts(const SgApp app, SgActiveContexts* pContexts);


#endif
