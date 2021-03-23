#ifndef SG_REND_H
#define SG_REND_H

#include "sage_base.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include "sg_math.h"

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

typedef struct SgResourceSetCreateInfo {
	SgResource**         ppResources;
	uint32_t             resourceCount;
	uint32_t             setIndex;
} SgResourceSetCreateInfo;

SgResult sgCreateResourceSet(const SgApp* pApp, const SgResourceSetCreateInfo *pCreateInfo, SgResourceSet **ppSgResourceSet);

typedef struct SgResourceSetInitInfo {
	SgResource**                  ppResources;
	uint32_t                      resourceCount;
	SgGraphicsInstance*           pGraphicsInstance; 

	SgBool                        isMeshResourceSet;
	uint32_t                      meshResourceSetID;
} SgResourceSetInitInfo;

SgResult sgInitResourceSet(const SgApp *pApp, SgResourceSetInitInfo *pInitInfo, SgResourceSet** ppResourceSet);

typedef struct SgShaderCreateInfo {
	SgFile*               pFile;
	SgShaderStageFlags    stage;
}SgShaderCreateInfo;

SgResult sgCreateShader(const SgApp *pApp, const SgShaderCreateInfo *pCreateInfo, SgShader **ppShader);

typedef struct SgSwapchainCreateInfo {
	VkSwapchainKHR oldSwapchain;
	VkRenderPass   renderPass;
} SgSwapchainCreateInfo;

SgResult sgCreateSwapchain(const SgApp *pApp, SgSwapchainCreateInfo *pCreateInfo, SgSwapchain *pSwapchain);

typedef struct SgGraphicsInstanceCreateInfo {
	SgShader**            ppShaders;
	uint32_t              shaderCount;

	SgResourceSet**       ppSets;
	uint32_t              setCount;

	SgResourceSet**       ppMeshSets;
	uint32_t              meshSetCount;
} SgGraphicsInstanceCreateInfo;

SgResult sgCreateGraphicsInstance(const SgApp *pApp, const SgGraphicsInstanceCreateInfo *pCreateInfo, SgGraphicsInstance **ppGraphicsInstance);

typedef struct SgUpdateCommandsInitInfo {
	SgApp*                 pApp;
	SgGraphicsInstance*    pGraphicsInstance;
	SgResource**           ppIndexResources;
	uint32_t               indexResourceCount;
} SgUpdateCommandsInitInfo;

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands** ppUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp*                 pApp;
	SgGraphicsInstance*    pGraphicsInstance;
	SgUpdateCommands*      pUpdateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo);

void sgDestroyShader(const SgApp *pApp, SgShader **ppShader);
void sgDestroyResource(const SgApp *pApp, SgResource **ppResource);
void sgDestroyResourceSet(const SgApp *pApp, SgResourceSet** ppResourceSet);
void sgDeinitUpdateCommands(const SgApp *pApp, SgUpdateCommands** ppUpdateCommands);
void sgDestroyGraphicsInstance(const SgApp *pApp, SgGraphicsInstance **ppGraphicsInstance);
void sgDestroyApp(SgApp **ppApp);

#endif
