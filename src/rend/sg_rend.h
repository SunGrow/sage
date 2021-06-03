#ifndef SG_REND_H
#define SG_REND_H

#include "sg_base.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "../res/sg_material.h"
#include "../res/sg_mesh.h"
#include "GLFW/glfw3.h"
#include "hashmap.h"
#include "sg_math.h"

typedef struct SgAppCreateInfo {
	const char* pName;
	SgFile*     pConfigFile;
	v2          size;
} SgAppCreateInfo;

SgResult  sgCreateApp(const SgAppCreateInfo* pCreateInfo, SgApp** ppApp);
SgWindow* sgGetWindow(SgApp* pApp);

typedef struct SgShaderCreateInfo {
	SgFile*            pFile;
	SgShaderStageFlags stage;
} SgShaderCreateInfo;

SgResult sgCreateShader(const SgApp*              pApp,
                        const SgShaderCreateInfo* pCreateInfo,
                        SgShader**                ppShader);

typedef struct SgUpdateCommandsInitInfo {
	SgMaterialMap* pMaterialMap;
	SgResourceMap* pResourceMap;
	SgMeshSet*     pMeshSet;
} SgUpdateCommandsInitInfo;

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo* pInitInfo,
                              SgUpdateCommands**              ppUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp*            pApp;
	SgMaterialMap*    pMaterialMap;
	SgMeshSet*        pMeshSet;
	SgUpdateCommands* pUpdateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(SgAppUpdateInfo* pUpdateInfo);

void sgDestroyShader(const SgApp* pApp, SgShader** ppShader);
void sgDeinitUpdateCommands(const SgApp*       pApp,
                            SgUpdateCommands** ppUpdateCommands);
void sgDestroyApp(SgApp** ppApp);

#endif
