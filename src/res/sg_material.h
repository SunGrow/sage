#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "../rend/command/sg_rend_command.h"
#include "../rend/resource/sg_resource.h"
#include "hashmap.h"
#include "sg_base.h"
#include "sg_math.h"

typedef struct SgRenderObject {
	uint32_t meshID;  // ID to get a mesh from set
	uint32_t instanceCount;
} SgRenderObject;

typedef struct SgRenderObjectCreateInfo {
	SgRenderObject* pRenderObjects;
	uint32_t renderObjectCount;
	const char** ppResourceNames;
	uint32_t resourceCount;

	const char* materialName;
	const char* pName;
} SgRenderObjectCreateInfo;

typedef struct SgRenderObjectUpdateInfo {
	SgRenderObject* pRenderObjects;
	uint32_t renderObjectCount;

	const char* pName;
} SgRenderObjectUpdateInfo;

typedef struct SgMaterialRenderObjects {
	SgRenderObject* pRenderObjects;
	uint32_t renderObjectCount;
	const char** ppResourceNames;
	uint32_t resourceCount;

	VkDescriptorSet* pDescriptorSets;
	uint32_t descriptorSetCount;

	VkWriteDescriptorSet* pWriteDescriptorSets;
	const char* materialName;
	const char* pName;
} SgMaterialRenderObjects;

typedef struct SgMaterialCreateInfo {
	const char* pMaterialName;
	SgResourceBinding* pResourceBindings;
	uint32_t resourceBindingCount;
	SgShader** ppShaders;
	uint32_t shaderCount;

	uint32_t renderObjectCount;
} SgMaterialCreateInfo;

typedef struct SgSetLayouts {
	VkDescriptorSetLayout* pSetLayouts;
	uint32_t setLayoutCount;
} SgSetLayouts;

typedef struct SgMaterial {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	SgSetLayouts setLayouts;
	uint32_t descriptorSetCount;

	uint32_t renderObjectCount;

	SgShader** ppShaders;
	uint32_t shaderCount;
	const char* pName;

	uint32_t resourceBindingCount;
	SgResourceBinding* pResourceBindings;
} SgMaterial;

typedef struct SgMaterialMap {
	struct hashmap* pMaterialRenderObjectMap;
	struct hashmap* pMaterialMap;
	uint32_t materialCount;

	VkDescriptorPool descriptorPool;

	SgSwapchain swapchain;
	VkRenderPass renderPass;
	const SgResourceMap* pResourceMap;
	const SgApp* pApp;
} SgMaterialMap;

typedef struct SgSwapchainCreateInfo {
	VkSwapchainKHR oldSwapchain;
	VkRenderPass renderPass;
} SgSwapchainCreateInfo;

SgResult sgCreateSwapchain(SgApp* pApp,
                           SgSwapchainCreateInfo* pCreateInfo,
                           SgSwapchain* pSwapchain);
void sgCleanupSwapchain(SgApp* pApp, SgSwapchain* pSwapchain);

SgResult sgBuildGraphicsPipeline(
    const SgApp* pApp,
    const SgGraphicsPipelineBuilder* pPipelineBuilder,
    VkRenderPass renderPass,
    VkPipeline* pPipeline);
SgResult sgBuildComputePipeline(const SgApp* pApp,
                                SgComputePipelineBuilder* pPipelineBuilder,
                                VkPipeline* pPipeline);

typedef struct SgMaterialMapCreateInfo {
	const SgResourceMap* pResourceMap;
	uint32_t materailCount;
} SgMaterialMapCreateInfo;

SgResult sgCreateMaterialMap(SgApp* pApp,
                             const SgMaterialMapCreateInfo* pCreateInfo,
                             SgMaterialMap** ppMaterialMap);
SgMaterial* sgAddMaterial(const SgMaterialCreateInfo* pCreateInfo,
                          SgMaterialMap** ppMaterialMap);
SgMaterial* sgGetMaterial(const char* pMaterialName,
                          const SgMaterialMap* ppMaterialMap);
SgResult sgAddMaterialRenderObjects(
    const SgRenderObjectCreateInfo* pRenderObjectsCreateInfo,
    SgMaterialMap** ppMaterialMap);
void sgUpdateRenderObjects(const SgRenderObjectUpdateInfo* pUpdateInfo,
                           SgMaterialMap** ppMaterialMap);
SgResult sgWriteMaterialRenderObjects(SgMaterialMap** ppMaterialMap);
// Will create/recreate descriptor set that could fit all of the material
// descriptor sets
SgResult sgInitMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap);

void sgDestroyMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap);

#endif
