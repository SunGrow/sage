#ifndef SAGE_INPUT_H
#define SAGE_INPUT_H

#include "sage_core.h"
#include "sage_rend.h"

SG_DEFINE_HANDLE(SgActiveContexts);
typedef void* SgActor;
typedef void (*SgInputAction)(/* State */ SgBool,
                              /* rangeX */ double,
                              /* rangeY */ double,
                              SgActor,
                              SgWindow);

typedef enum SgInputType {
	SG_INPUT_TYPE_KEYBOARD = BIT(0),
	SG_INPUT_TYPE_MOUSE    = BIT(1),
} SgInputType;

typedef struct SgInputSignal {
	SgSize key;
	SgSize mod;

	char* keyName;
	char* modName;

	SgInputType type;
} SgInputSignal;

typedef struct SgActiveContextsCreateInfo {
	SgActor* pActors;
	char**   pActorNames;
	SgSize   actorCount;

	SgInputSignal* pInputSignals;
	SgSize         signalCount;

	SgInputAction* pActionFuncs;
	char**         pActionNames;
	SgSize         actionCount;

	SgFile file;
} SgActiveContextsCreateInfo;

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo,
                        SgActiveContexts*                 pContexts);
void     sgUpdateContext(const SgActiveContextsCreateInfo* pCreateInfo,
                         SgActiveContexts*                 pContexts);

typedef enum SgActionType {
	SG_ACTION_TYPE_TRIGGER = BIT(0),
	SG_ACTION_TYPE_TOGGLE  = BIT(1),
	SG_ACTION_TYPE_RANGE   = BIT(2),
} SgActionType;

typedef struct SgActionNames {
	SgActionType actionType;

	SgInputType inputType;
	char*       inputName;
	char*       modName;
	char*       actionName;
	SgSize      actorID;
} SgActionNames;

typedef struct SgActiveContextsChangeInfo {
	SgActionNames* pOldActions;
	SgActionNames* pNewActions;
	SgSize         count;
} SgActiveContextsChangeInfo;

void sgChangeContext(const SgActiveContextsChangeInfo* pChangeInfo,
                     SgActiveContexts*                 pContexts);
void sgSaveContext(const SgActiveContexts contexts, char* fileDir);

void sgSetActiveContexts(SgActiveContexts activeContexts, SgApp* pApp);

void sgUnloadContexts(const SgApp app, SgActiveContexts* pContexts);

#endif
