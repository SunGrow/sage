#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "sage_base.h"
#include "sg_math.h"
#include "hashmap.h"
#include "../rend/resource/sg_resource.h"
#include "../rend/command/sg_rend_command.h"

typedef struct SgRenderObject {
	uint32_t meshID; // ID to get a mesh from set
	uint32_t instanceCount;
} SgRenderObject;

typedef struct SgRenderObjectCreateInfo {
	SgRenderObject*          pRenderObjects;
	uint32_t                 renderObjectCount;
	SgResource**             ppResources;
	uint32_t                 resourceCount;

	const char*              materialName; // ID to be addressed in a map
	const char*              pName;
} SgRenderObjectCreateInfo;

typedef struct SgRenderObjectUpdateInfo {
	SgRenderObject*          pRenderObjects;
	uint32_t                 renderObjectCount;

	const char*              pName;
} SgRenderObjectUpdateInfo;

typedef struct SgMaterialRenderObjects {
	SgRenderObject*        pRenderObjects;
	uint32_t               renderObjectCount;
	SgResource**           ppResources;
	uint32_t               resourceCount;

	VkDescriptorSet*       pDescriptorSets;
	uint32_t               descriptorSetCount;

	VkWriteDescriptorSet*  pWriteDescriptorSets;
	const char*            materialName; 
	const char*            pName;
} SgMaterialRenderObjects;

typedef struct SgMaterialCreateInfo {
	const char*            pMaterialName;
	SgResourceBinding*     pResourceBindings;
	uint32_t               resourceBindingCount;
	SgShader**             ppShaders;
	uint32_t               shaderCount;

	uint32_t               renderObjectCount;
} SgMaterialCreateInfo;

typedef struct SgSetLayouts {
	VkDescriptorSetLayout* pSetLayouts;
	uint32_t               setLayoutCount;
} SgSetLayouts;

typedef struct SgMaterial {
	VkPipeline             pipeline;
	VkPipelineLayout       pipelineLayout;
	SgSetLayouts           setLayouts;
	uint32_t               descriptorSetCount;

	uint32_t               renderObjectCount;

	SgShader**             ppShaders;
	uint32_t               shaderCount;
	const char*            pName;

	uint32_t               resourceBindingCount;
	SgResourceBinding*     pResourceBindings;
} SgMaterial;

typedef struct SgMaterialMap {
	struct hashmap*    pMaterialRenderObjectMap;
	struct hashmap*    pMaterialMap;
	uint32_t           materialCount;

	VkDescriptorPool   descriptorPool;

	SgSwapchain        swapchain;
	VkRenderPass       renderPass;
	SgApp*             pApp;
} SgMaterialMap;


SgResult sgBuildGraphicsPipeline(const SgApp* pApp, const SgGraphicsPipelineBuilder* pPipelineBuilder, VkRenderPass renderPass, VkPipeline* pPipeline);
SgResult sgBuildComputePipeline(const SgApp* pApp, SgComputePipelineBuilder* pPipelineBuilder, VkPipeline* pPipeline);

SgResult sgCreateMaterialMap(SgApp* pApp, uint32_t materialCount, SgMaterialMap** ppMaterialMap);
SgMaterial* sgAddMaterial(const SgMaterialCreateInfo* pCreateInfo, SgMaterialMap** ppMaterialMap);
SgMaterial* sgGetMaterial(const char* pMaterialName, const SgMaterialMap* ppMaterialMap);
SgResult sgAddMaterialRenderObjects(const SgRenderObjectCreateInfo* pRenderObjectsCreateInfo, SgMaterialMap** ppMaterialMap);
void sgUpdateRenderObjects(const SgRenderObjectUpdateInfo* pUpdateInfo, SgMaterialMap** ppMaterialMap);
SgResult sgWriteMaterialRenderObjects(SgMaterialMap** ppMaterialMap);
// Will create/recreate descriptor set that could fit all of the material descriptor sets
SgResult sgInitMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap);

void sgDestroyMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap);


#endif
