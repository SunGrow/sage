#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "sage_base.h"
#include "sg_math.h"
#include "hashmap.h"
#include "../rend/resource/sg_resource.h"
#include "../rend/command/sg_rend_command.h"

typedef struct SgRenderObject {
	uint32_t meshID; // ID to get a mesh from set
	m4       transformMatrix;
} SgRenderObject;

typedef struct SgRenderObjectCreateInfo {
	SgRenderObject* pRenderObjects;
	uint32_t        renderObjectCount;
	const char*     materialObjectsName;
	SgResource**    ppResources;
	uint32_t*       pResourceSetBindings;
	uint32_t        resourceCount;
	uint32_t        resourceSetCount;

	const char*     materialName; // ID to be addressed in a map
} SgRenderObjectCreateInfo;

typedef struct SgMaterialRenderObjects {
	SgRenderObject*        pRenderObjects;
	uint32_t               renderObjectCount;
	const char*            materialObjectsName;
	SgResource**           ppResources;
	uint32_t*              pResourceSetBindings;
	uint32_t               resourceCount;
	uint32_t               resourceSetCount;
	VkWriteDescriptorSet*  pWriteDescriptorSets;
	const char*            materialName; // ID to be addressed in a map
} SgMaterialRenderObjects;

typedef struct SgMaterialCreateInfo {
	const char*            pMaterialName;
	SgResourceBinding**    ppResourceBindings;
	uint32_t*              pResourceBindingCount;
	uint32_t               resourceSetBindingCount;
	SgShader**             ppShaders;
	uint32_t               shaderCount;
} SgMaterialCreateInfo;

typedef struct SgSetLayouts {
	VkDescriptorSetLayout* setLayouts;
	uint32_t               setLayoutCount;
} SgSetLayouts;

typedef struct SgMaterial {
	VkPipeline             pipeline;
	VkPipelineLayout       pipelineLayout;
	SgSetLayouts           setLayouts;
	VkDescriptorSet*       pDescriptorSets;
	uint32_t               descriptorSetCount;

	SgShader**             ppShaders;
	uint32_t               shaderCount;
	const char*            pName;

	uint32_t*              pResourceBindingCount;
	uint32_t               resourceSetBindingCount;
	SgResourceBinding**    ppResourceBindings;
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
SgResult sgWriteMaterialRenderObjects(SgMaterialMap** ppMaterialMap);
// Will create/recreate descriptor set that could fit all of the material descriptor sets and consequentially fill it up with descriptor sets
SgResult sgInitMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap);


#endif
