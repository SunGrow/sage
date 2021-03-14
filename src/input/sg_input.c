#include "sg_input.h"
#include "cJSON.h"
#include "hashmap.h"
#include "stdlib.h"
#include "string.h"


//# File Structure:
//	{
//		contexts : [
//			{ # context
//				actorNames : ["mainScene"],
//				priority : 0, # 0 - highest
//				name : "",
//				toggleMap : [
//					{type : "", input : "", mod : "", actionName : "", actorID : 0},
//					{type : "", input : "", mod : "", actionName : "", actorID : 0}, 
//				],
//				triggerMap : [
//					{type : "", input : "", mod : "", actionName : "", actorID : 0},
//					{type : "", input : "", mod : "", actionName : "", actorID : 0}, 
//				],
//			},
//		]
//	}
//

int keyCompare(const void* a, const void* b, void* udata) {
	const SgInputKey* keyA = a;
	const SgInputKey* keyB = b;

	uint32_t keyTypeNeq = keyA->type != keyB->type;
	uint32_t keyNeq = keyA->key != keyB->key;
	uint32_t keyModNeq = keyA->mods != keyB->mods;

	return keyTypeNeq * (keyA->type - keyB->type) 
		+ (!keyTypeNeq) * (keyNeq) * (keyA->key - keyB->key) 
		+ (!keyTypeNeq) * (!keyNeq) * (keyModNeq) * (keyA->type - keyB->type);
}

uint64_t keyHash(const void *item, uint64_t seed0, uint64_t seed1) {
	struct tmpKey {
	int32_t key;
	int32_t mods;
	SgInputType type;
	};
    const struct tmpKey* key = item;
    return hashmap_sip(key, sizeof(*key), seed0, seed1);
}


bool keyIter(const void *item, void *udata) {
    const SgInputKey* key = item;
	log_info("[JSON]: hashmap item is [key: %d, mods: %d, type: %d, id: %d]", key->key, key->mods, key->type, key->id);
    return true;
}

static SgResult sgFillActionMap(const SgActiveContextsCreateInfo* pCreateInfo, cJSON* actionMap, SgActionMap* pActionMap) {
	pActionMap->type = SG_ACTION_TYPE_TOGGLE;
	pActionMap->actionCount = cJSON_GetArraySize(actionMap);
	SG_CALLOC_NUM(pActionMap->actorIDs, pActionMap->actionCount);
	SG_CALLOC_NUM(pActionMap->actionFuncs, pActionMap->actionCount);
	SG_CALLOC_NUM(pActionMap->states, pActionMap->actionCount);

	pActionMap->actionMap = hashmap_new(sizeof(SgInputKey), pActionMap->actionCount, 0, 0, keyHash, keyCompare, NULL);

	for (uint32_t j = 0; j < cJSON_GetArraySize(actionMap); ++j) {
		cJSON* action = cJSON_GetArrayItem(actionMap, j);

		SgInputKey inputKey;

		cJSON* actionName = cJSON_GetObjectItem(action, "actionName");
		for (uint32_t k = 0; k < pCreateInfo->actionCount; ++k) {
			if (strcmp(pCreateInfo->pActionNames[k], actionName->valuestring) == SG_SUCCESS) {
				pActionMap->actionFuncs[j] = pCreateInfo->pActionFuncs[k];
			}
		}
		cJSON* actorID = cJSON_GetObjectItem(action, "actorID");
		pActionMap->actorIDs[j] = actorID->valueint;

		cJSON* type = cJSON_GetObjectItem(action, "type");
		// TODO: Replace if..else if..else with an array iteration if possible
		if (strcmp(type->valuestring, SG_INPUT_TYPE_KEYBOARD_NAME) == SG_SUCCESS) {
			inputKey.type = SG_INPUT_TYPE_KEYBOARD;
		} else if (strcmp(type->valuestring, SG_INPUT_TYPE_MOUSE_NAME) == SG_SUCCESS) {
			inputKey.type = SG_INPUT_TYPE_MOUSE;
		}
		cJSON* inputName = cJSON_GetObjectItem(action, "input");
		cJSON* modName = cJSON_GetObjectItem(action, "mod");
		if (strlen(inputName->valuestring)) {
			for (uint32_t k = 0; k < pCreateInfo->signalCount; ++k) {
				if (pCreateInfo->pInputSignals[k].keyName) {
					if (strcmp(pCreateInfo->pInputSignals[k].keyName, inputName->valuestring) == SG_SUCCESS) {
						inputKey.key = pCreateInfo->pInputSignals[k].key;
					}
				}
			}
		} else {
			inputKey.key = 0;
		}
		if (strlen(modName->valuestring)) {
			for (uint32_t k = 0; k < pCreateInfo->signalCount; ++k) {
				if (pCreateInfo->pInputSignals[k].modName) {
					if (strcmp(pCreateInfo->pInputSignals[k].modName, modName->valuestring) == SG_SUCCESS) {
						inputKey.mods = pCreateInfo->pInputSignals[k].mod;
					}
				}
			}
		} else {
			inputKey.mods = 0;
		}
		inputKey.id = j;
		hashmap_set(pActionMap->actionMap, &inputKey);
	}
	hashmap_scan(pActionMap->actionMap, keyIter, NULL);

	return SG_SUCCESS;
}

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts** ppContexts) {
	cJSON* contexts_json = cJSON_Parse((char*) pCreateInfo->pFile->pBytes);
	if (contexts_json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            log_error("[JSON]: Error before: %s\n", error_ptr);
			return -1;
        }
    }
	log_info("[JSON]: YES");
	cJSON* contexts = cJSON_GetObjectItem(contexts_json, "contexts");

	SgActiveContexts* pActiveContexts;
	SG_CALLOC_NUM(pActiveContexts, 1);
	pActiveContexts->contextCount = cJSON_GetArraySize(contexts);
	if (pActiveContexts->contextCount == 0) {
		log_warn("[JSON]: Active context is empty");
		return -1;
	}
	SG_CALLOC_NUM(pActiveContexts->pContexts, pActiveContexts->contextCount);
	for (uint32_t i = 0; i < cJSON_GetArraySize(contexts); ++i) {
		cJSON* context = cJSON_GetArrayItem(contexts, i);

		cJSON* name = cJSON_GetObjectItem(context, "name");
		log_info("[JSON]: Context Name: \"%s\"", name->valuestring);

		cJSON* actorNames = cJSON_GetObjectItem(context, "actorNames");
		SG_CALLOC_NUM(pActiveContexts->pContexts[i].pActors, cJSON_GetArraySize(actorNames));
		pActiveContexts->pContexts[i].actorCount = cJSON_GetArraySize(actorNames);
		for (uint32_t j = 0; j < cJSON_GetArraySize(actorNames); ++j) {
			cJSON* actorName = cJSON_GetArrayItem(actorNames, j);
			for (uint32_t k = 0; k < pCreateInfo->actorCount; ++k) {
				if (strcmp(actorName->valuestring, pCreateInfo->pActorNames[k]) == SG_SUCCESS) {
					pActiveContexts->pContexts[i].pActors[j] = pCreateInfo->pActors[k];
					log_info("[JSON]: Actor %s found", actorName->valuestring);
				}
			}
			log_info("[JSON]: Actor Name: \"%s\"", actorName->valuestring);
		}
		cJSON* toggleMap = cJSON_GetObjectItem(context, "toggleMap");
		sgFillActionMap(pCreateInfo, toggleMap, &pActiveContexts->pContexts[i].toggleMap);

		cJSON* triggerMap = cJSON_GetObjectItem(context, "triggerMap");
		sgFillActionMap(pCreateInfo, triggerMap, &pActiveContexts->pContexts[i].triggerMap);

		cJSON* rangeMap = cJSON_GetObjectItem(context, "rangeMap");
		sgFillActionMap(pCreateInfo, rangeMap, &pActiveContexts->pContexts[i].rangeMap);
	}

	*ppContexts = pActiveContexts;

	return SG_SUCCESS;
}

static void sgCallActions(SgActiveContexts* pActiveContexts, SgInputType inputType, SgActionType actionType, int key, int mods, int actions, float rangeX, float rangeY, GLFWwindow *pWindow) {
	SgActionMap activeMap = {0};
	for (uint32_t i = 0; i < pActiveContexts->contextCount; ++i) {
		if ((actionType == SG_ACTION_TYPE_TRIGGER) && (actions == GLFW_PRESS || actions == GLFW_RELEASE)) {
			activeMap = pActiveContexts->pContexts[i].triggerMap;
		} else if ((actionType == SG_ACTION_TYPE_TOGGLE) && (actions == GLFW_REPEAT || actions == GLFW_RELEASE)) {
			activeMap = pActiveContexts->pContexts[i].toggleMap;
		} else if (actionType == SG_ACTION_TYPE_RANGE) {
			activeMap = pActiveContexts->pContexts[i].rangeMap;
		} else {
			continue;
		}
		if (actions == GLFW_PRESS || actions == GLFW_RELEASE || actionType == SG_ACTION_TYPE_RANGE) {
			SgInputKey* inputKey = hashmap_get(activeMap.actionMap, &(SgInputKey) {.key = key, .mods = mods, .type = inputType});
			if (inputKey) {
				uint32_t actorID = activeMap.actorIDs[inputKey->id];
				SgActor actor = pActiveContexts->pContexts[i].pActors[actorID];
				SgInputAction action = activeMap.actionFuncs[inputKey->id];
				action(actions, rangeX, rangeY, actor, pWindow);
			}
		}
	}
}

static void onKeyboardKey(GLFWwindow *pWindow, int key, int scancode, int actions, int mods)
{
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_KEYBOARD, SG_ACTION_TYPE_TRIGGER, key, mods, actions, 0, 0, pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_KEYBOARD, SG_ACTION_TYPE_TOGGLE, key, mods, actions, 0, 0, pWindow);
}

static void onMouseKey(GLFWwindow *pWindow, int key, int actions, int mods) {
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_TRIGGER, key, mods, actions, 0, 0, pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_TOGGLE, key, mods, actions, 0, 0, pWindow);
}

static void onCursorPosition(GLFWwindow *pWindow, double xPosition, double yPosition) {
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_RANGE, 0, 0, 0, xPosition, yPosition, pWindow);
}

void sgSetActiveContexts(SgActiveContexts *pActiveContexts, SgApp **ppApp) {
	SgApp* pApp = *ppApp;
	glfwSetWindowUserPointer(pApp->pWindow, pActiveContexts);
	glfwSetKeyCallback(pApp->pWindow, onKeyboardKey);
	glfwSetMouseButtonCallback(pApp->pWindow, onMouseKey);
	glfwSetCursorPosCallback(pApp->pWindow, onCursorPosition);
}
