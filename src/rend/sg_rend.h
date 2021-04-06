#ifndef SG_REND_H
#define SG_REND_H

#include "sage_base.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "sg_math.h"
#include "../res/sg_mesh.h"
#include "../res/sg_material.h"
#include "hashmap.h"

typedef enum SgAppCreateFlagBits {
	SG_APP_CURSOR_HIDDEN         = 0x00000001,
	SG_APP_WINDOW_RESIZABLE      = 0x00000002,
	SG_APP_WINDOW_FULLSCREEN     = 0x00000004,
} SgAppCreateFlagBits;
typedef SgFlags SgAppCreateFlags;

typedef struct SgAppCreateInfo {
	const char*            pName;
	SgAppCreateFlags       flags;
	v2                     size;
} SgAppCreateInfo;

SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp **ppApp);
SgWindow* sgGetWindow(SgApp* pApp);

typedef struct SgShaderCreateInfo {
	SgFile*               pFile;
	SgShaderStageFlags    stage;
}SgShaderCreateInfo;

SgResult sgCreateShader(const SgApp *pApp, const SgShaderCreateInfo *pCreateInfo, SgShader **ppShader);

typedef struct SgUpdateCommandsInitInfo {
	SgMaterialMap*         pMaterialMap;
	SgMeshSet*             pMeshSet;
} SgUpdateCommandsInitInfo;

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands** ppUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp*                 pApp;
	SgMaterialMap*         pMaterialMap;
	SgMeshSet*             pMeshSet;
	SgUpdateCommands*      pUpdateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(SgAppUpdateInfo* pUpdateInfo);

void sgDestroyShader(const SgApp *pApp, SgShader **ppShader);
void sgDestroyResource(const SgApp *pApp, SgResource **ppResource);
void sgDeinitUpdateCommands(const SgApp *pApp, SgUpdateCommands** ppUpdateCommands);
void sgDestroyApp(SgApp **ppApp);

#endif
