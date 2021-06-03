#ifndef SAGE_REND_H_
#define SAGE_REND_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
#include "hashmap.h"
#include "sage_core.h"
#include "sage_math.h"
#include "sage_window.h"
#include "stb_image.h"
#define VK_NO_PROTOTYPES
#include "volk.h"
#define GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

typedef struct SgAppCreateInfo {
	const char* pName;
	SgFile      configFile;
} SgAppCreateInfo;

SG_DEFINE_HANDLE(SgApp);
SgResult sgCreateApp(const SgAppCreateInfo* pCreateInfo, SgApp* pSgApp);
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
	SgFile             file;
	SgShaderStageFlags stage;
} SgShaderCreateInfo;

SG_DEFINE_HANDLE(SgShader);
SgResult sgCreateShader(const SgApp               app,
                        const SgShaderCreateInfo* pCreateInfo,
                        SgShader*                 pShader);

typedef enum SgResourceTypeFlagBits {
	SG_RESOURCE_TYPE_TEXTURE_2D = BIT(0),
	SG_RESOURCE_TYPE_MESH       = BIT(1),
	SG_RESOURCE_TYPE_UNIFORM    = BIT(2),
	SG_RESOURCE_TYPE_INDICES    = BIT(3),
} SgResourceTypeFlagBits;
typedef SgFlags SgResourceTypeFlags;

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	void*               bytes;
	SgSize              size;
	VkExtent3D          extent;
	SgSize              layerCount;
	SgSize              levelCount;

	const char* pName;
} SgResourceCreateInfo;

SG_DEFINE_HANDLE(SgResourceMap);
SgResult sgCreateResourceMap(const SgApp app, SgResourceMap* pResourceMap);
SgResult sgAddResource(const SgApp                 app,
                       const SgResourceCreateInfo* pCreateInfo,
                       SgResourceMap*              pResourceMap);
SgResult sgDestroyResourceMap(const SgApp app, SgResourceMap* pResourceMap);
SgResult sgInitResourceMap(const SgApp app, SgResourceMap* pResourceMap);

/* Make mesh load fit with the engine theme */
typedef struct SgVertex {
	v3 vert;
	u8 norm[4];
	v2 tex;
} SgVertex;

typedef struct SgMeshSet {
	SgVertex*       pVertices;
	SgSize          vertexCount;
	SgSize*         pVertexOffsets;
	SgSize*         pIndexSizes;
	SgSize*         pVertexSizes;
	SgSize*         pIndices;
	SgSize          indexCount;
	SgSize*         pIndexOffsets;
	SgSize          meshCount;
	struct hashmap* meshMap;  // Return an offset id

	const char* indicesResourceName;
} SgMeshSet;

SgResult sgCreateMeshSet(SgMeshSet** ppMeshArray);
SgSize   sgAddMesh(const char* pPath, SgMeshSet** ppMeshArray);

SgSize* sgGetMeshID(const char* pPath, const SgMeshSet* pMeshArray);
void    sgUnloadMesh(SgMeshSet** ppMesh);

typedef struct SgTexture {
	int32_t  width, height, channels;
	SgSize   size;
	stbi_uc* pixels;
} SgTexture;

void sgLoadTexture(const char* pPath, SgTexture** pTexture);
void sgUnloadTexture(SgTexture** ppTexture);

//////

typedef struct SgRenderObject {
	SgSize meshID;  // ID to get a mesh from set
	SgSize instanceCount;
} SgRenderObject;

typedef struct SgResourceBinding {
	SgResourceTypeFlags type;
	SgShaderStageFlags  stage;
	SgSize              binding;
	SgSize              setBinding;
} SgResourceBinding;

typedef struct SgRenderObjectCreateInfo {
	SgRenderObject* pRenderObjects;
	SgSize          renderObjectCount;
	const char**    ppResourceNames;
	SgSize          resourceCount;

	const char* materialName;
	const char* pName;
} SgRenderObjectCreateInfo;

SG_DEFINE_HANDLE(SgMaterialRenderObjects);

typedef struct SgMaterialCreateInfo {
	const char*        pMaterialName;
	SgResourceBinding* pResourceBindings;
	SgSize             resourceBindingCount;
	SgShader*          pShaders;
	SgSize             shaderCount;

	SgSize renderObjectCount;
} SgMaterialCreateInfo;

typedef struct SgSetLayouts {
	VkDescriptorSetLayout* setLayouts;
	SgSize                 setLayoutCount;
} SgSetLayouts;

SG_DEFINE_HANDLE(SgMaterial);
SG_DEFINE_HANDLE(SgMaterialMap);

typedef struct SgMaterialMapCreateInfo {
	const SgResourceMap resourceMap;
	SgSize              materailCount;
} SgMaterialMapCreateInfo;

SgResult   sgCreateMaterialMap(SgApp                          app,
                               const SgMaterialMapCreateInfo* pCreateInfo,
                               SgMaterialMap*                 pMaterialMap);
SgMaterial sgAddMaterial(const SgMaterialCreateInfo* pCreateInfo,
                         SgMaterialMap*              pMaterialMap);
SgMaterial sgGetMaterial(const char*         pMaterialName,
                         const SgMaterialMap materialMap);
SgResult   sgAddMaterialRenderObjects(
      const SgRenderObjectCreateInfo* pRenderObjectsCreateInfo,
      SgMaterialMap*                  pMaterialMap);
typedef struct SgRenderObjectUpdateInfo {
	SgRenderObject* pRenderObjects;
	SgSize          renderObjectCount;

	const char* pName;
} SgRenderObjectUpdateInfo;
void     sgUpdateRenderObjects(const SgRenderObjectUpdateInfo* pUpdateInfo,
                               SgMaterialMap*                  pMaterialMap);
SgResult sgWriteMaterialRenderObjects(SgMaterialMap* pMaterialMap);
// Will create/recreate descriptor set that could fit all of the material
// descriptor sets and consequentially fill it up with descriptor sets
SgResult sgInitMaterialMap(SgApp app, SgMaterialMap* pMaterialMap);

void sgDestroyMaterialMap(SgApp app, SgMaterialMap* pMaterialMap);

typedef struct SgData {
	void*  bytes;
	size_t size;
} SgData;
SgResult sgUpdateResource(const SgApp   app,
                          SgResourceMap resourceMap,
                          const SgData* pData,
                          const char*   pName);
SgResult sgUpdateResources(const SgApp    app,
                           SgResourceMap  resourceMap,
                           const SgSize   resourceCount,
                           const SgData** ppData,
                           const char**   ppNames);
// Update resources with last assigned data (the pointer is stored, so you
// better not have destroyed the last used data)
SgResult sgUpdateAllResources(const SgApp app, SgResourceMap resourceMap);

typedef struct SgUpdateCommandsInitInfo {
	SgMaterialMap materialMap;
	SgResourceMap resourceMap;
	SgMeshSet*    pMeshSet;
} SgUpdateCommandsInitInfo;

SG_DEFINE_HANDLE(SgUpdateCommands);
SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo* pInitInfo,
                              SgUpdateCommands*               pUpdateCommands);

typedef struct SgAppUpdateInfo {
	SgApp            app;
	SgMaterialMap    materialMap;
	SgMeshSet*       pMeshSet;
	SgUpdateCommands updateCommands;
} SgAppUpdateInfo;
SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo);

void sgDestroyShader(const SgApp app, SgShader* pShader);
void sgDeinitUpdateCommands(const SgApp app, SgUpdateCommands* pUpdateCommands);
void sgDestroyApp(SgApp* pApp);

#ifdef __cplusplus
}
#endif

#endif
