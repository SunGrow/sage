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
void sgUpdateContext(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts* pContexts);

typedef enum SgActionType {
	SG_ACTION_TYPE_TRIGGER = BIT(0),
	SG_ACTION_TYPE_TOGGLE  = BIT(1),
	SG_ACTION_TYPE_RANGE   = BIT(2),
} SgActionType;

typedef struct SgActionNames {
	SgActionType actionType;

	SgInputType inputType;
	char* inputName;
	char* modName;
	char* actionName;
} SgActionNames;

typedef struct SgActiveContextsChangeInfo {
	SgActionNames* pOldActions;
	SgActionNames* pNewActions;
	uint32_t       count;
} SgActiveContextsChangeInfo;

void sgChangeContext(const SgActiveContextsChangeInfo* pChangeInfo, SgActiveContexts* pContexts);
void sgSaveContext(const SgActiveContexts contexts, char* fileDir);

void sgSetActiveContexts(SgActiveContexts activeContexts, SgApp *pApp);

void sgUnloadContexts(const SgApp app, SgActiveContexts* pContexts);



#endif
