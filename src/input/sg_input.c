#include "sg_input.h"
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

static int keyCompare(const void* a, const void* b, void* udata) {
	const SgInputKey* keyA = a;
	const SgInputKey* keyB = b;

	uint32_t keyTypeNeq = keyA->type != keyB->type;
	uint32_t keyNeq = keyA->key != keyB->key;
	uint32_t keyModNeq = keyA->mods != keyB->mods;

	return keyTypeNeq * (keyA->type - keyB->type) 
		+ (!keyTypeNeq) * (keyNeq) * (keyA->key - keyB->key) 
		+ (!keyTypeNeq) * (!keyNeq) * (keyModNeq) * (keyA->type - keyB->type);
}

static uint64_t keyHash(const void *item, uint64_t seed0, uint64_t seed1) {
	struct tmpKey {
		int32_t key;
		int32_t mods;
		SgInputType type;
	};
    const struct tmpKey* key = item;
    return hashmap_sip(key, sizeof(*key), seed0, seed1);
}


static bool keyIter(const void *item, void *udata) {
    const SgInputKey* key = item;
	log_info("[JSON]: hashmap item is [key: %d, mods: %d, type: %d, id: %d]", key->key, key->mods, key->type, key->id);
    return true;
}

static SgResult sgFillActionMap(const SgActiveContextsCreateInfo* pCreateInfo, cJSON* actionMap, SgActionMap* pActionMap) {
	pActionMap->type = SG_ACTION_TYPE_TOGGLE;
	pActionMap->actionCount = cJSON_GetArraySize(actionMap);
	if (pActionMap->actorIDs == NULL) {
		SG_CALLOC_NUM(pActionMap->actorIDs, pActionMap->actionCount);
	}
	if (pActionMap->actionFuncs == NULL) {
		SG_CALLOC_NUM(pActionMap->actionFuncs, pActionMap->actionCount);
	}
	if (pActionMap->states == NULL) {
		SG_CALLOC_NUM(pActionMap->states, pActionMap->actionCount);
	}

	if (pActionMap->actionMap != NULL) {
		hashmap_free(pActionMap->actionMap);
	}
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

	return SG_SUCCESS;
}

void sgUpdateContext(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts** ppContexts) {
	SgActiveContexts* pActiveContexts = *ppContexts;
	cJSON* contexts = cJSON_GetObjectItem(pActiveContexts->contextsJSON, "contexts");
	if (pActiveContexts->pContexts == NULL) {
		SG_CALLOC_NUM(pActiveContexts->pContexts, pActiveContexts->contextCount);
	}
	for (uint32_t i = 0; i < cJSON_GetArraySize(contexts); ++i) {
		cJSON* context = cJSON_GetArrayItem(contexts, i);

		cJSON* actorNames = cJSON_GetObjectItem(context, "actorNames");
		if (pActiveContexts->pContexts[i].pActors == NULL) {
			SG_CALLOC_NUM(pActiveContexts->pContexts[i].pActors, cJSON_GetArraySize(actorNames));
		}
		pActiveContexts->pContexts[i].actorCount = cJSON_GetArraySize(actorNames);
		for (uint32_t j = 0; j < cJSON_GetArraySize(actorNames); ++j) {
			cJSON* actorName = cJSON_GetArrayItem(actorNames, j);
			for (uint32_t k = 0; k < pCreateInfo->actorCount; ++k) {
				if (strcmp(actorName->valuestring, pCreateInfo->pActorNames[k]) == SG_SUCCESS) {
					pActiveContexts->pContexts[i].pActors[j] = pCreateInfo->pActors[k];
				}
			}
		}
		cJSON* toggleMap = cJSON_GetObjectItem(context, "toggleMap");
		sgFillActionMap(pCreateInfo, toggleMap, &pActiveContexts->pContexts[i].toggleMap);

		cJSON* triggerMap = cJSON_GetObjectItem(context, "triggerMap");
		sgFillActionMap(pCreateInfo, triggerMap, &pActiveContexts->pContexts[i].triggerMap);

		cJSON* rangeMap = cJSON_GetObjectItem(context, "rangeMap");
		sgFillActionMap(pCreateInfo, rangeMap, &pActiveContexts->pContexts[i].rangeMap);
	}

	*ppContexts = pActiveContexts;
	return;
}

SgResult sgLoadContexts(const SgActiveContextsCreateInfo* pCreateInfo, SgActiveContexts** ppContexts) {
	const char *error_ptr;
	cJSON* contexts_json = cJSON_ParseWithOpts((char*) pCreateInfo->pFile->pBytes, &error_ptr, 1);
	if (contexts_json == NULL) {
        if (error_ptr != NULL) {
            log_error("[JSON]: Error before: %s\n", error_ptr);
			return -1;
        }
    }
	cJSON* contexts = cJSON_GetObjectItem(contexts_json, "contexts");

	SgActiveContexts* pActiveContexts;
	SG_CALLOC_NUM(pActiveContexts, 1);
	pActiveContexts->contextCount = cJSON_GetArraySize(contexts);
	if (pActiveContexts->contextCount == 0) {
		log_warn("[JSON]: Active context is empty");
		return -1;
	}
	pActiveContexts->contextsJSON = contexts_json;

	sgUpdateContext(pCreateInfo, &pActiveContexts);
	*ppContexts = pActiveContexts;
	return SG_SUCCESS;
}

static void sgCallActions(SgActiveContexts* pActiveContexts, SgInputType inputType, SgActionType actionType, int key, int mods, int actions, double rangeX, double rangeY, GLFWwindow *pWindow) {
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
			// A botch
			else if (actions == GLFW_RELEASE) {
				inputKey = hashmap_get(activeMap.actionMap, &(SgInputKey) {.key = key, .mods = 0, .type = inputType});
				if (inputKey) {
					uint32_t actorID = activeMap.actorIDs[inputKey->id];
					SgActor actor = pActiveContexts->pContexts[i].pActors[actorID];
					SgInputAction action = activeMap.actionFuncs[inputKey->id];
					action(actions, rangeX, rangeY, actor, pWindow);
				}
			}
		}
	}
}

static void onKeyboardKey(GLFWwindow *pWindow, int key, int scancode, int actions, int mods)
{
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	if (!pActiveContexts) {
		return;
	}
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_KEYBOARD, SG_ACTION_TYPE_TRIGGER, key, mods, actions, 0, 0, pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_KEYBOARD, SG_ACTION_TYPE_TOGGLE, key, mods, actions, 0, 0, pWindow);
}

static void onMouseKey(GLFWwindow *pWindow, int key, int actions, int mods) {
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	if (!pActiveContexts) {
		return;
	}
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_TRIGGER, key, mods, actions, 0, 0, pWindow);
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_TOGGLE, key, mods, actions, 0, 0, pWindow);
}

static void onCursorPosition(GLFWwindow *pWindow, double xPosition, double yPosition) {
    SgActiveContexts* pActiveContexts = glfwGetWindowUserPointer(pWindow);
	if (!pActiveContexts) {
		return;
	}
	sgCallActions(pActiveContexts, SG_INPUT_TYPE_MOUSE, SG_ACTION_TYPE_RANGE, 0, 0, 0, xPosition, yPosition, pWindow);
}

void sgSetActiveContexts(SgActiveContexts *pActiveContexts, SgApp **ppApp) {
	SgApp* pApp = *ppApp;
	glfwSetWindowUserPointer(pApp->pWindow, pActiveContexts);
	glfwSetKeyCallback(pApp->pWindow, onKeyboardKey);
	glfwSetMouseButtonCallback(pApp->pWindow, onMouseKey);
	glfwSetCursorPosCallback(pApp->pWindow, onCursorPosition);
}

static void sgUnloadMap(SgActionMap *pActionMap) {
	hashmap_free(pActionMap->actionMap);
	if (pActionMap->actionFuncs) {
		free(pActionMap->actionFuncs);
		pActionMap->actionCount = 0;
	}
	if (pActionMap->actorIDs) {
		free(pActionMap->actorIDs);
	}
	if (pActionMap->states) {
		free(pActionMap->states);
	}
}

void sgUnloadContexts(const SgApp *pApp, SgActiveContexts** ppContexts) {
	SgActiveContexts* pContexts = *ppContexts;
	if (pContexts->pContexts) {
		for (uint32_t i = 0; i < pContexts->contextCount; ++i) {
			sgUnloadMap(&pContexts->pContexts[i].triggerMap);
			sgUnloadMap(&pContexts->pContexts[i].toggleMap);
			sgUnloadMap(&pContexts->pContexts[i].rangeMap);
			if (pContexts->pContexts[i].pActors) {
				free(pContexts->pContexts[i].pActors);
				pContexts->pContexts[i].actorCount = 0;
			}
		}
		free(pContexts->pContexts);
	}
	pContexts->contextCount = 0;
	cJSON_Delete(pContexts->contextsJSON);
	free(pContexts);
	ppContexts = NULL;
	glfwSetWindowUserPointer(pApp->pWindow, NULL);
	return;
}

static void sgReplaceActions(const SgActiveContextsChangeInfo* pChangeInfo, const SgActionType actionType, cJSON** ppActionMap) {
	cJSON* pActionMap = *ppActionMap;

	for (uint32_t i = 0; i < cJSON_GetArraySize(pActionMap); ++i) {
		cJSON* action = cJSON_GetArrayItem(pActionMap, i);

		cJSON* inputType    = cJSON_GetObjectItem(action, "type");
		cJSON* inputK       = cJSON_GetObjectItem(action, "input");
		cJSON* mod          = cJSON_GetObjectItem(action, "mod");
		cJSON* actionFunc   = cJSON_GetObjectItem(action, "actionName");
		char* inputTypeName = cJSON_GetStringValue(inputType);
		char* inputName     = cJSON_GetStringValue(inputK);
		char* modName       = cJSON_GetStringValue(mod);
		char* actionName    = cJSON_GetStringValue(actionFunc);

		for (uint32_t j = 0; j < pChangeInfo->count; ++j) {
			SgBool notTheOne = actionType != pChangeInfo->pOldActions[j].actionType;
			notTheOne |= strcmp(pChangeInfo->pOldActions[j].actionName, actionName);
			notTheOne |= strcmp(pChangeInfo->pOldActions[j].modName, modName);
			notTheOne |= strcmp(pChangeInfo->pOldActions[j].inputName, inputName);
			char* cmpInputTypeName;
			switch(pChangeInfo->pOldActions[j].inputType) {
				case(SG_INPUT_TYPE_KEYBOARD):
					cmpInputTypeName = SG_INPUT_TYPE_KEYBOARD_NAME;
					break;
				case(SG_INPUT_TYPE_MOUSE):
					cmpInputTypeName = SG_INPUT_TYPE_MOUSE_NAME;
					break;
			};
			notTheOne |= strcmp(cmpInputTypeName, inputTypeName);

			if (notTheOne) {
				continue;
			}

			cJSON_SetValuestring(inputK, pChangeInfo->pNewActions[j].inputName);
			cJSON_SetValuestring(actionFunc, pChangeInfo->pNewActions[j].actionName);
			cJSON_SetValuestring(mod, pChangeInfo->pNewActions[j].modName);
			switch(pChangeInfo->pNewActions[j].inputType) {
				case(SG_INPUT_TYPE_KEYBOARD):
					cmpInputTypeName = SG_INPUT_TYPE_KEYBOARD_NAME;
					break;
				case(SG_INPUT_TYPE_MOUSE):
					cmpInputTypeName = SG_INPUT_TYPE_MOUSE_NAME;
					break;
			};
			cJSON_SetValuestring(inputType, cmpInputTypeName);
		}
	}

	*ppActionMap = pActionMap;
}

void sgChangeContext(const SgActiveContextsChangeInfo* pChangeInfo, SgActiveContexts** ppContexts) {
	SgActiveContexts* pContexts = *ppContexts;
	cJSON* contexts = cJSON_GetObjectItem(pContexts->contextsJSON, "contexts");
	for (uint32_t i = 0; i < cJSON_GetArraySize(contexts); ++i) {
		cJSON* context = cJSON_GetArrayItem(contexts, i);
		cJSON* toggleMap = cJSON_GetObjectItem(context, "toggleMap");
		sgReplaceActions(pChangeInfo, SG_ACTION_TYPE_TOGGLE, &toggleMap);
		cJSON* triggerMap = cJSON_GetObjectItem(context, "triggerMap");
		sgReplaceActions(pChangeInfo, SG_ACTION_TYPE_TRIGGER, &triggerMap);
		cJSON* rangeMap = cJSON_GetObjectItem(context, "rangeMap");
		sgReplaceActions(pChangeInfo, SG_ACTION_TYPE_RANGE, &rangeMap);
	}
	*ppContexts = pContexts;
}

void sgSaveContext(const SgActiveContexts* pContexts, char* fileDir) {
	char * json = cJSON_Print(pContexts->contextsJSON);
	SgFile file = {
		.pBytes = (uint32_t*)json,
		.size = strlen(json),
	};
	sgWriteFile(fileDir, &file);
}
