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

SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp **ppSgApp);

typedef struct SgResourceSetCreateInfo {
	SgResource**         ppResources;
	uint32_t             resourceCount;
	uint32_t             setIndex;
} SgResourceSetCreateInfo;

SgResult sgCreateResourceSet(const SgApp* pApp, const SgResourceSetCreateInfo *pCreateInfo, SgResourceSet **ppSgResourceSet);

typedef struct SgResourceSetInitInfo {
	SgResourceSet*                pResourceSet;
	SgResource**                  ppResources;
	uint32_t                      resourceCount;
	SgGraphicsInstance*           pGraphicsInstance; 
} SgResourceSetInitInfo;

SgResult sgInitResourceSet(const SgApp *pApp, const SgResourceSetInitInfo *pInitInfo);

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
} SgGraphicsInstanceCreateInfo;

SgResult sgCreateGraphicsInstance(const SgApp *pApp, const SgGraphicsInstanceCreateInfo *pCreateInfo, SgGraphicsInstance **ppGraphicsInstance);

typedef struct SgAppUpdateInfo {
	SgApp*                 pApp;
	SgGraphicsInstance*    pGraphicsInstance;
	SgResource**           ppIndexResouces;
	uint32_t               indexResourceCount;
} SgAppUpdateInfo;
SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo);

void sgDestroyApp(SgApp **ppApp);

#endif
