#ifndef SG_INPUT_H
#define SG_INPUT_H

#include "GLFW/glfw3.h"
#include "cJSON.h"
#include "hashmap.h"
#include "log.h"
#include "sg_base.h"

typedef struct SgInput {
	SgSize actorID;
	SgBool state;
} SgInput;

typedef void* SgActor;
typedef void (*SgInputAction)(SgBool, double, double, SgActor, SgWindow*);

#define SG_INPUT_TYPE_KEYBOARD_NAME "keyboard"
#define SG_INPUT_TYPE_MOUSE_NAME "mouse"

typedef enum SgActionType {
	SG_ACTION_TYPE_TRIGGER = BIT(0),
	SG_ACTION_TYPE_TOGGLE  = BIT(1),
	SG_ACTION_TYPE_RANGE   = BIT(2),
} SgActionType;

typedef enum SgInputType {
	SG_INPUT_TYPE_KEYBOARD = BIT(0),
	SG_INPUT_TYPE_MOUSE    = BIT(1),
} SgInputType;

typedef struct SgInputKey {
	int32_t key;
	int32_t mods;
	SgInputType type;
	uint64_t id;
} SgInputKey;

typedef struct SgActionMap {
	struct hashmap* actionMap;  // Map from inputs to actions

	SgInputType* types;
	SgSize* actorIDs;
	SgInputAction* actionFuncs;
	SgBool* states;

	SgSize actionCount;
	SgActionType type;
} SgActionMap;

typedef struct SgContext {
	SgActionMap triggerMap;
	SgActionMap toggleMap;
	SgActionMap rangeMap;

	SgActor* pActors;   // Map addresed by actorID
	SgSize actorCount;  // Not really needed. Debug
} SgContext;

typedef struct SgInputSignal {
	SgSize key;
	SgSize mod;

	char* keyName;
	char* modName;

	SgInputType type;
} SgInputSignal;

typedef struct SgActiveContexts {
	SgContext* pContexts;
	SgSize contextCount;

	cJSON* contextsJSON;
} SgActiveContexts;

typedef struct SgActiveContextsCreateInfo {
	SgActor* pActors;
	char** pActorNames;
	SgSize actorCount;

	SgInputSignal* pInputSignals;
	SgSize signalCount;

	SgInputAction* pActionFuncs;
	char** pActionNames;
	SgSize actionCount;

	SgFile* pFile;
} SgActiveContextsCreateInfo;

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo,
                        SgActiveContexts** ppContexts);
void sgUpdateContext(const SgActiveContextsCreateInfo* pCreateInfo,
                     SgActiveContexts** ppContexts);

typedef struct SgActionNames {
	SgActionType actionType;

	SgInputType inputType;
	char* inputName;
	char* modName;
	char* actionName;
	SgSize actorID;
} SgActionNames;

typedef struct SgActiveContextsChangeInfo {
	SgActionNames* pOldActions;
	SgActionNames* pNewActions;
	SgSize count;
} SgActiveContextsChangeInfo;

void sgChangeContext(const SgActiveContextsChangeInfo* pChangeInfo,
                     SgActiveContexts** ppContexts);
void sgSaveContext(const SgActiveContexts* pContexts, char* fileDir);

void sgSetActiveContexts(SgActiveContexts* pActiveContexts, SgApp** ppApp);

void sgUnloadContexts(const SgApp* pApp, SgActiveContexts** ppContexts);

#endif
