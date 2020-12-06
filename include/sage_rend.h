#ifndef SAGE_REND_H_
#define SAGE_REND_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
#include "sage_core.h"
#include "sage_math.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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


SG_DEFINE_HANDLE(SgApp);
SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp *pSgApp);

typedef enum SgShaderStageFlagBits {
    SG_SHADER_STAGE_VERTEX_BIT   = VK_SHADER_STAGE_VERTEX_BIT,
    SG_SHADER_STAGE_GEOMETRY_BIT = VK_SHADER_STAGE_GEOMETRY_BIT,
    SG_SHADER_STAGE_FRAGMENT_BIT = VK_SHADER_STAGE_FRAGMENT_BIT,
    SG_SHADER_STAGE_COMPUTE_BIT  = VK_SHADER_STAGE_COMPUTE_BIT,
} SgShaderStageFlagBits;
typedef SgFlags SgShaderStageFlags;
typedef struct SgShaderCreateInfo {
	SgFile                file;
	SgShaderStageFlags    stage;
}SgShaderCreateInfo;

SG_DEFINE_HANDLE(SgShader);
SgResult sgCreateShader(const SgApp pApp, const SgShaderCreateInfo* pCreateInfo, SgShader *pShader);

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D   = 0x00000001,
	SG_RESOURCE_TYPE_MESH         = 0x00000002,
	SG_RESOURCE_TYPE_UNIFORM      = 0x00000004,
	SG_RESOURCE_TYPE_INDICES      = 0x00000008,
} SgResourceTypeFlagBits;
typedef SgFlags SgResourceTypeFlags;

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	SgShaderStageFlags  stage;
	void*               bytes;
	uint32_t            size;
	uint32_t            binding;
} SgResourceCreateInfo;

SG_DEFINE_HANDLE(SgResource);
SgResult sgCreateResource(const SgApp app, const SgResourceCreateInfo *pCreateInfo, SgResource *pResource);

typedef struct SgResourceSetCreateInfo {
	SgResource*          pResources;
	uint32_t             resourceCount;
	uint32_t             setIndex;
} SgResourceSetCreateInfo;
SG_DEFINE_HANDLE(SgResourceSet)
SgResult sgCreateResourceSet(const SgApp app, const SgResourceSetCreateInfo *pCreateInfo, SgResourceSet *pSgResourceSet);

SG_DEFINE_HANDLE(SgResourceSet);
typedef struct SgGraphicsInstanceCreateInfo {
	SgShader*             pShaders;
	uint32_t              shaderCount;

	SgResourceSet*        pSets;
	uint32_t              setCount;

	SgResourceSet*        pMeshSets;
	uint32_t              meshSetCount;
} SgGraphicsInstanceCreateInfo;

SG_DEFINE_HANDLE(SgGraphicsInstance);
SgResult sgCreateGraphicsInstance(const SgApp app, const SgGraphicsInstanceCreateInfo *pCreateInfo, SgGraphicsInstance *pGraphicsInstance);

typedef struct SgData {
	void*             bytes;
	size_t            size;
} SgData;
SgResult sgUpdateResource(const SgApp app, SgData* pData, SgResource* pResource);

typedef struct SgResourceSetInitInfo {
	SgResource*                   pResources;
	uint32_t                      resourceCount;
	SgGraphicsInstance            graphicsInstance; 

	SgBool                        isMeshResourceSet;
	uint32_t                      meshResourceSetID;
} SgResourceSetInitInfo;

SgResult sgInitResourceSet(const SgApp app, const SgResourceSetInitInfo *pInitInfo, SgResourceSet* pResourceSet);

typedef struct SgUpdateCommandsInitInfo {
	SgApp                  app;
	SgGraphicsInstance     graphicsInstance;
	SgResource*            pIndexResouces;
	uint32_t               indexResourceCount;
} SgUpdateCommandsInitInfo;

SG_DEFINE_HANDLE(SgUpdateCommands);
SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands* pUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp                  app;
	SgGraphicsInstance     graphicsInstance;
	SgUpdateCommands       updateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo);

void sgDestroyApp(SgApp *pApp);

#ifdef __cplusplus
}
#endif

#endif
