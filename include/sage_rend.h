#ifndef SAGE_REND_H_
#define SAGE_REND_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
#include "sage_core.h"
#include "sage_window.h"
#include "sage_math.h"
#include "stb_image.h"
#include "hashmap.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
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
SgWindow sgGetWindow(SgApp app);
/**/

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
SgResult sgCreateShader(const SgApp app, const SgShaderCreateInfo* pCreateInfo, SgShader *pShader);

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D   = BIT(0),
	SG_RESOURCE_TYPE_MESH         = BIT(1),
	SG_RESOURCE_TYPE_UNIFORM      = BIT(2),
	SG_RESOURCE_TYPE_INDICES      = BIT(3),
} SgResourceTypeFlagBits;
typedef SgFlags SgResourceTypeFlags;

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	void*               bytes;
	uint32_t            size;
	VkExtent3D          extent;
} SgResourceCreateInfo;

SG_DEFINE_HANDLE(SgResource);
SgResult sgCreateResource(const SgApp app, const SgResourceCreateInfo *pCreateInfo, SgResource *pResource);

/* Make mesh load fit with the engine theme */
typedef struct SgVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgVertex;

typedef struct SgMeshSet {
	SgVertex*          pVertices;
	uint32_t           vertexCount;
	uint32_t*          pVertexOffsets;
	uint32_t*          pIndexSizes;
	uint32_t*          pVertexSizes;
	uint32_t*          pIndices;
	uint32_t           indexCount;
	uint32_t*          pIndexOffsets;
	uint32_t           meshCount;
	struct hashmap*    meshMap;      // Return an offset id

	SgResource         indicesResource;
} SgMeshSet;

SgResult  sgCreateMeshSet(SgMeshSet** ppMeshArray);
uint32_t  sgAddMesh(const char* pPath, SgMeshSet** ppMeshArray);

uint32_t* sgGetMeshID(const char* pPath, const SgMeshSet* pMeshArray);
void      sgUnloadMesh(SgMeshSet **ppMesh);

typedef struct SgMeshTransformInfo {
	v3 move;
	v3 scale;
} SgMeshTransformInfo;

typedef struct SgTexture {
	int32_t width, height, channels;
	uint32_t size;
	stbi_uc *pixels;
} SgTexture;

void sgLoadTexture(const char *pPath, SgTexture **pTexture);
void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t offset, uint32_t vertCount, SgVertex *pVertices);
void sgUnloadTexture(SgTexture **ppTexture);

//////

typedef struct SgRenderObject {
	uint32_t meshID; // ID to get a mesh from set
	uint32_t instanceCount;
} SgRenderObject;

typedef struct SgResourceBinding {
	SgResourceTypeFlags          type;
	SgShaderStageFlags           stage;
	uint32_t                     binding;
	uint32_t                     setBinding;
} SgResourceBinding;

typedef struct SgRenderObjectCreateInfo {
	SgRenderObject*          pRenderObjects;
	uint32_t                 renderObjectCount;
	SgResource*              pResources;
	uint32_t                 resourceCount;

	const char*              materialName;
	const char*              pName;
} SgRenderObjectCreateInfo;

SG_DEFINE_HANDLE(SgMaterialRenderObjects);

typedef struct SgMaterialCreateInfo {
	const char*            pMaterialName;
	SgResourceBinding*     pResourceBindings;
	uint32_t               resourceBindingCount;
	SgShader*              pShaders;
	uint32_t               shaderCount;

	uint32_t               renderObjectCount;
} SgMaterialCreateInfo;

typedef struct SgSetLayouts {
	VkDescriptorSetLayout* setLayouts;
	uint32_t               setLayoutCount;
} SgSetLayouts;

SG_DEFINE_HANDLE(SgMaterial);
SG_DEFINE_HANDLE(SgMaterialMap);

SgResult sgCreateMaterialMap(const SgApp app, uint32_t materialCount, SgMaterialMap* pMaterialMap);
SgMaterial sgAddMaterial(const SgMaterialCreateInfo* pCreateInfo, SgMaterialMap* pMaterialMap);
SgMaterial sgGetMaterial(const char* pMaterialName, const SgMaterialMap materialMap);
SgResult sgAddMaterialRenderObjects(const SgRenderObjectCreateInfo* pRenderObjectsCreateInfo, SgMaterialMap* pMaterialMap);
typedef struct SgRenderObjectUpdateInfo {
	SgRenderObject*          pRenderObjects;
	uint32_t                 renderObjectCount;

	const char*              pName;
} SgRenderObjectUpdateInfo;
void sgUpdateRenderObjects(const SgRenderObjectUpdateInfo* pUpdateInfo, SgMaterialMap* pMaterialMap);
SgResult sgWriteMaterialRenderObjects(SgMaterialMap* pMaterialMap);
// Will create/recreate descriptor set that could fit all of the material descriptor sets and consequentially fill it up with descriptor sets
SgResult sgInitMaterialMap(SgApp app, SgMaterialMap* pMaterialMap);

void sgDestroyMaterialMap(SgApp app, SgMaterialMap* pMaterialMap);

typedef struct SgData {
	void*             bytes;
	size_t            size;
} SgData;
SgResult sgUpdateResource(const SgApp app, SgData* pData, SgResource* pResource);

typedef struct SgUpdateCommandsInitInfo {
	SgMaterialMap          materialMap;
	SgMeshSet*             pMeshSet;
} SgUpdateCommandsInitInfo;

SG_DEFINE_HANDLE(SgUpdateCommands);
SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands* pUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp                  app;
	SgMaterialMap          materialMap;
	SgMeshSet*             pMeshSet;
	SgUpdateCommands       updateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo);

void sgDestroyShader(const SgApp app, SgShader *pShader);
void sgDestroyResource(const SgApp app, SgResource *pResource);
void sgDeinitUpdateCommands(const SgApp app, SgUpdateCommands* pUpdateCommands);
void sgDestroyApp(SgApp *pApp);

#ifdef __cplusplus
}
#endif

#endif
