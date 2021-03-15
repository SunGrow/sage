#ifndef SG_INPUT_H
#define SG_INPUT_H

#include "sage_base.h"
#include "GLFW/glfw3.h"
#include "hashmap.h"
#include "log.h"

typedef struct SgInput {
	uint32_t actorID;
	uint32_t signalID; // Mb don't need it
	SgBool   state;
} SgInput;

typedef void* SgActor;
typedef void (*SgInputAction) (uint32_t, double, double, SgActor, GLFWwindow*);

#define SG_INPUT_TYPE_KEYBOARD_NAME "keyboard"
#define SG_INPUT_TYPE_MOUSE_NAME    "mouse"

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
	struct hashmap*       actionMap; // Map from inputs to actions

	SgInputType*          types;
	uint32_t*             actorIDs;
	SgInputAction*        actionFuncs;
	SgBool*               states;

	uint32_t              actionCount;
	SgActionType          type;
} SgActionMap;

typedef struct SgContext {
	SgActionMap           triggerMap;
	SgActionMap           toggleMap;

	SgActionMap           rangeMap;

	SgActor*              pActors;      // Map addresed by actorID
	uint32_t              actorCount;   // Not really needed. Debug
} SgContext;

typedef struct SgInputSignal {
	uint32_t          key;
	uint32_t          mod;

	char*             keyName;
	char*             modName;

	SgInputType       type;
} SgInputSignal;

typedef struct SgActiveContexts {
	SgContext* pContexts;
	uint32_t   contextCount;
} SgActiveContexts;

typedef struct SgActiveContextsCreateInfo {
	SgActor*          pActors;
	char**            pActorNames;
	uint32_t          actorCount;

	SgInputSignal*    pInputSignals;
	uint32_t          signalCount;

	SgInputAction*    pActionFuncs;
	char**            pActionNames;
	uint32_t          actionCount;

	SgFile*           pFile;
} SgActiveContextsCreateInfo;

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts** ppContexts);

void sgSetActiveContexts(SgActiveContexts *pActiveContexts, SgApp **ppApp);

void sgUnloadContexts(const SgApp *pApp, SgActiveContexts** ppContexts);


#endif
