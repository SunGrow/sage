#include "sg_material.h"
#include <stdlib.h>
#include <string.h>

static int materialRenderObjectsKeyCompare(const void* a, const void* b, void* udata) {
	const SgMaterialRenderObjects* keyA = a;
	const SgMaterialRenderObjects* keyB = b;
	return strcmp(keyA->materialObjectsName, keyB->materialObjectsName);
}

static uint64_t materialRenderObjectsKeyHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SgMaterialRenderObjects* key = item;
    return hashmap_sip(key->materialObjectsName, strlen(key->materialObjectsName), seed0, seed1);
}

static int materialKeyCompare(const void* a, const void* b, void* udata) {
	const SgMaterial* keyA = a;
	const SgMaterial* keyB = b;
	return strcmp(keyA->pName, keyB->pName);
}

static uint64_t materialKeyHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SgMaterial* key = item;
    return hashmap_sip(key->pName, strlen(key->pName), seed0, seed1);
}

static SgResult sgFillDefaultRenderpass(const SgApp* pApp, VkRenderPass* pRenderPass) {
	VkDevice device = pApp->device;
	VkFormat format = pApp->surfaceAttributes.format.format;

	VkAttachmentDescription attachments[] = {
	    {
	        .format = format,
	        .samples = pApp->msaaSampleCount,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	    },
	    {
			.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
			.samples = pApp->msaaSampleCount,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    },
		{
			.format = format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
	};

	VkAttachmentReference colorAttachmentRef = {
	    .attachment = 0,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference depthAttachmentRef = {
	    .attachment = 1,
	    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference resolveAttachmentRef = {
	    .attachment = 2,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
	    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	    .colorAttachmentCount = 1,
	    .pColorAttachments = &colorAttachmentRef,
	    .pDepthStencilAttachment = &depthAttachmentRef,
		.pResolveAttachments = &resolveAttachmentRef,
	};

	VkSubpassDependency pDependencies[] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,

			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		},
	};

	VkRenderPassCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,

		.pAttachments = attachments,
		.attachmentCount = NUMOF(attachments),

		.pSubpasses = &subpass,
		.subpassCount = 1,

		.pDependencies = pDependencies,
		.dependencyCount = NUMOF(pDependencies),
	};

	if(vkCreateRenderPass(device, &createInfo, 0, pRenderPass) == VK_SUCCESS) {
		sgLogInfo_Debug("[Graphics Instance]: Render Pass Created");
	} else {
		sgLogError("[Graphics Instance]: Render Pass Creation Failure");
	}

	return SG_SUCCESS;
}

SgResult createVkSwapchain(const SgApp* pApp, VkSwapchainKHR oldswapchain, VkSwapchainKHR* pSwapchain) {
	VkSurfaceCapabilitiesKHR surfcap = pApp->surfaceAttributes.surfaceCapabilities;
	VkCompositeAlphaFlagBitsKHR surfaceComposite =
	    (surfcap.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	        ? VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
	        : (surfcap.supportedCompositeAlpha &
	           VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
	              ? VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR
	              : (surfcap.supportedCompositeAlpha &
	                 VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
	                    ? VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR
	                    : VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	VkSwapchainCreateInfoKHR createinfo = {
	    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	    .surface = pApp->surface,
	    .minImageCount = SG_FRAME_QUEUE_LENGTH,
	    .imageFormat = pApp->surfaceAttributes.format.format,
	    .imageColorSpace = pApp->surfaceAttributes.format.colorSpace,
	    .imageExtent = pApp->surfaceAttributes.surfaceCapabilities.currentExtent,
	    .imageArrayLayers = 1,
	    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
	    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	    .queueFamilyIndexCount = 1,
	    .pQueueFamilyIndices = &pApp->graphicsQueueFamilyIdx,
	    .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
	    .compositeAlpha = surfaceComposite,
	    .presentMode = pApp->surfaceAttributes.presentMode,
	    .oldSwapchain = oldswapchain,
	};
	if (vkCreateSwapchainKHR(pApp->device, &createinfo, VK_NULL_HANDLE, pSwapchain) == VK_SUCCESS) {
	} else {
		sgLogError("[Graphics Instance]: Swapchain Creation Failure");
		return 1;
	}
	return SG_SUCCESS;
}

typedef struct SgSwapchainCreateInfo {
	VkSwapchainKHR oldSwapchain;
	VkRenderPass   renderPass;
} SgSwapchainCreateInfo;

SgResult sgCreateSwapchain(const SgApp *pApp, SgSwapchainCreateInfo *pCreateInfo, SgSwapchain *pSwapchain) {

	createVkSwapchain(pApp, pCreateInfo->oldSwapchain, &pSwapchain->swapchain);
	pSwapchain->extent = pApp->surfaceAttributes.surfaceCapabilities.currentExtent;

	/* Frame Image Creation */
	vkGetSwapchainImagesKHR(pApp->device, pSwapchain->swapchain, &pSwapchain->imageCount, VK_NULL_HANDLE);
	SG_MALLOC_NUM(pSwapchain->pFrameImages, pSwapchain->imageCount);
	vkGetSwapchainImagesKHR(pApp->device, pSwapchain->swapchain, &pSwapchain->imageCount, pSwapchain->pFrameImages);
	SG_MALLOC_NUM(pSwapchain->pFrameImageViews, pSwapchain->imageCount);

	SgImageViewCreateInfo imageViewCreateInfo = {
		.type = VK_IMAGE_VIEW_TYPE_2D,
		.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
	};
	for (uint32_t i = 0; i < pSwapchain->imageCount; ++i) {
		SgImage image = {
			.image = pSwapchain->pFrameImages[i],
			.format = pApp->surfaceAttributes.format.format,
		};
		SgImageView imageView;
		imageViewCreateInfo.pImage = &image;
		sgCreateImageView(pApp, &imageViewCreateInfo, &imageView);
		pSwapchain->pFrameImageViews[i] = imageView.imageView;
	}

	/* Depth Image Creation */
	SgImageCreateInfo depthImageCreateInfo = {
		.extent = (VkExtent3D) {pSwapchain->extent.width, pSwapchain->extent.height, 1},
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.type = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT_S8_UINT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.layout = VK_IMAGE_LAYOUT_UNDEFINED,
		.samples = pApp->msaaSampleCount,
	};
	sgCreateImage(pApp, &depthImageCreateInfo, &pSwapchain->depthImage);

	SgImageViewCreateInfo depthImageViewCreateInfo = {
		.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
		.pImage = &pSwapchain->depthImage,
		.type = VK_IMAGE_VIEW_TYPE_2D,
	};
	sgCreateImageView(pApp, &depthImageViewCreateInfo, &pSwapchain->depthImageView);
	/* Transition depth image */
	{
		SgImageTransferInfo transferInfo = {
			.image = pSwapchain->depthImage,
			.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.dstImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		};
		sgTransferImage(pApp, &transferInfo);
	}

	/**/

	/* Blend Image to Creation */
	SgImageCreateInfo blendImageCreateInfo = {
		.extent = (VkExtent3D) {pSwapchain->extent.width, pSwapchain->extent.height, 1},
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.type = VK_IMAGE_TYPE_2D,
		.format = pApp->surfaceAttributes.format.format,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.layout = VK_IMAGE_LAYOUT_UNDEFINED,
		.samples = pApp->msaaSampleCount,
	};
	sgCreateImage(pApp, &blendImageCreateInfo, &pSwapchain->blendImage);
	SgImageViewCreateInfo blendImageViewCreateInfo = {
		.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
		.pImage = &pSwapchain->blendImage,
		.type = VK_IMAGE_VIEW_TYPE_2D,
	};
	sgCreateImageView(pApp, &blendImageViewCreateInfo, &pSwapchain->blendImageView);

	/* FrameBuffer */
	VkFramebufferCreateInfo framebufferCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
	    .renderPass = pCreateInfo->renderPass,
	    .width = pSwapchain->extent.width,
	    .height = pSwapchain->extent.height,
	    .layers = 1,
	};
	SG_MALLOC_NUM(pSwapchain->pFrameBuffers, pSwapchain->imageCount);


	for (uint32_t i = 0; i < pSwapchain->imageCount; ++i) {
		VkImageView pAttachments[] = {pSwapchain->blendImageView.imageView, pSwapchain->depthImageView.imageView, pSwapchain->pFrameImageViews[i]};
		framebufferCreateInfo.attachmentCount = NUMOF(pAttachments);
		framebufferCreateInfo.pAttachments = pAttachments;

		if (vkCreateFramebuffer(pApp->device, &framebufferCreateInfo, VK_NULL_HANDLE, &pSwapchain->pFrameBuffers[i]) == VK_SUCCESS) {
		} else {
			sgLogError("[Graphics Instance]: Framebuffer %d Creation Failure", i);
		}
	}
	sgLogInfo_Debug("[Graphics Instance]: Framebuffer Creation Finished");
	return SG_SUCCESS;
}

SgResult sgCreateMaterialMap(SgApp* pApp, uint32_t materialCount, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	SG_CALLOC_NUM(pMaterialMap, 1);
	pMaterialMap->pMaterialRenderObjectMap = hashmap_new(sizeof(SgMaterialRenderObjects), materialCount, 0, 0, materialRenderObjectsKeyHash, materialRenderObjectsKeyCompare, NULL);
	pMaterialMap->pMaterialMap = hashmap_new(sizeof(SgMaterial), materialCount, 0, 0, materialKeyHash, materialKeyCompare, NULL);

	sgFillDefaultRenderpass(pApp, &pMaterialMap->renderPass);
	/* Create Swapchain */
	SgSwapchainCreateInfo swapchainCreateInfo = {
		.renderPass = pMaterialMap->renderPass,
	};
	sgCreateSwapchain(pApp, &swapchainCreateInfo, &pMaterialMap->swapchain);
	pMaterialMap->pApp = pApp;
	*ppMaterialMap = pMaterialMap;
	return SG_SUCCESS;
}

SgResult sgFillGraphicsPipelineBuilder(const SgApp* pApp, SgGraphicsPipelineBuilder* pPipelineBuilder) {
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};
	pPipelineBuilder->inputAssembly = inputAssembly;

	VkPipelineVertexInputStateCreateInfo vertexInput = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};
	pPipelineBuilder->vertexInputInfo = vertexInput;

	VkPipelineRasterizationStateCreateInfo rasterizationState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    .lineWidth = 1.f,
	};
	pPipelineBuilder->rasterizer = rasterizationState;

	VkPipelineColorBlendAttachmentState colorAttachmentState = {
		 .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,

		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};
	pPipelineBuilder->colorBlendAttachment = colorAttachmentState;


	VkPipelineMultisampleStateCreateInfo multiSampleState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = pApp->msaaSampleCount,
		.minSampleShading = 1.0f,
	};
	pPipelineBuilder->multisampling = multiSampleState;

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable = VK_TRUE,
	    .depthWriteEnable = VK_TRUE,
	    .depthCompareOp = VK_COMPARE_OP_LESS,
	};
	pPipelineBuilder->depthStencil = depthStencilState;

	return SG_SUCCESS;
}

static SgResult sgSetGraphicsPipelineBuilderStages(const SgMaterialCreateInfo* pCreateInfo, SgGraphicsPipelineBuilder* pPipelineBuilder) {
	SG_CALLOC_NUM(pPipelineBuilder->pShaderStages, pCreateInfo->shaderCount);
	pPipelineBuilder->shaderStageCount = pCreateInfo->shaderCount;
	for (uint32_t i = 0; i < pPipelineBuilder->shaderStageCount; ++i) {
		pPipelineBuilder->pShaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		pPipelineBuilder->pShaderStages[i].module = pCreateInfo->ppShaders[i]->shader;
		pPipelineBuilder->pShaderStages[i].stage = pCreateInfo->ppShaders[i]->stage;
		pPipelineBuilder->pShaderStages[i].pName = "main";
	}
	return SG_SUCCESS;
}

SgResult sgBuildGraphicsPipeline(const SgApp* pApp, const SgGraphicsPipelineBuilder* pPipelineBuilder, VkRenderPass renderPass, VkPipeline* pPipeline) {

	VkPipelineViewportStateCreateInfo viewportstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .scissorCount = 1,
	};
	VkDynamicState dynamicstates[] = {VK_DYNAMIC_STATE_VIEWPORT,
	                                  VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .dynamicStateCount = NUMOF(dynamicstates),
	    .pDynamicStates = dynamicstates,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_COPY,
	    .attachmentCount = 1,
	    .pAttachments = &pPipelineBuilder->colorBlendAttachment,
		.blendConstants[0] = 0.0f,
		.blendConstants[1] = 0.0f,
		.blendConstants[2] = 0.0f,
		.blendConstants[3] = 0.0f,
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

	    .stageCount = pPipelineBuilder->shaderStageCount,
	    .pStages = pPipelineBuilder->pShaderStages,

	    .pInputAssemblyState = &pPipelineBuilder->inputAssembly,
	    .pVertexInputState = &pPipelineBuilder->vertexInputInfo,

		.pViewportState = &viewportstate,
		.pRasterizationState = &pPipelineBuilder->rasterizer,
		.pMultisampleState = &pPipelineBuilder->multisampling,
		.pDepthStencilState = &pPipelineBuilder->depthStencil,
		.pColorBlendState = &colorBlendState,
		.pDynamicState = &dynamicstate,
		.layout = pPipelineBuilder->pipelineLayout,
		.renderPass = renderPass,
	};

	SgResult result = vkCreateGraphicsPipelines(pApp->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, 0, pPipeline);
	if (result) {
		sgLogError("[Pipeline]: Graphics Pipeline Creation failure");
	} else {
		sgLogInfo_Debug("[Pipeline]: Graphics Pipeline Creation successfull");
	}
	return SG_SUCCESS;
}

SgResult sgBuildComputePipeline(const SgApp* pApp, SgComputePipelineBuilder* pPipelineBuilder, VkPipeline* pPipeline) {
	// Not derived atm. MB later
	VkComputePipelineCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.layout = pPipelineBuilder->pipelineLayout,
		.stage = pPipelineBuilder->shaderStage,
	};
	SgResult result = vkCreateComputePipelines(pApp->device, VK_NULL_HANDLE, 1, &createInfo, VK_NULL_HANDLE, pPipeline);
	if (result) {
		sgLogError("[Pipeline]: Compute Pipeline Creation failure");
	} else {
		sgLogInfo_Debug("[Pipeline]: Compute Pipeline Creation successfull");
	}
	return SG_SUCCESS;
}

static SgResult sgCreateDescriptorSetLayout(const SgApp* pApp, const SgMaterialCreateInfo* pCreateInfo, SgSetLayouts* pSetLayouts) {
	VkDescriptorSetLayoutBinding** pSetLayoutBindings;
	SG_CALLOC_NUM(pSetLayoutBindings, pCreateInfo->resourceSetBindingCount);
	pSetLayouts->setLayoutCount = pCreateInfo->resourceSetBindingCount;
	SG_CALLOC_NUM(pSetLayouts->setLayouts, pSetLayouts->setLayoutCount);
	for (uint32_t i = 0; i < pSetLayouts->setLayoutCount; ++i) {
		SG_CALLOC_NUM(pSetLayoutBindings[i], pCreateInfo->pResourceBindingCount[i]);
		for (uint32_t j = 0; j < pCreateInfo->pResourceBindingCount[i]; ++j) {
			// Binding inside of a set
			pSetLayoutBindings[i][j].binding = pCreateInfo->ppResourceBindings[i][j].binding;
			pSetLayoutBindings[i][j].stageFlags = pCreateInfo->ppResourceBindings[i][j].stage;
			pSetLayoutBindings[i][j].descriptorCount = 1;
			switch (pCreateInfo->ppResourceBindings[i][j].type) {
				case (SG_RESOURCE_TYPE_INDICES):
					break;
				case (SG_RESOURCE_TYPE_MESH):
					pSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_UNIFORM):
					pSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_TEXTURE_2D):
					pSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
			}
		}
		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pBindings = pSetLayoutBindings[i],
			.bindingCount = pCreateInfo->pResourceBindingCount[i],
		};
		VkResult result = vkCreateDescriptorSetLayout(pApp->device, &setLayoutCreateInfo, VK_NULL_HANDLE, &pSetLayouts->setLayouts[i]);
		if(result == VK_SUCCESS) {
			sgLogInfo_Debug("[Set]: Descriptor Set Layout Init Successfull");
		} else {
			sgLogError("[Set]: Descriptor Set Layout Init Error");
		}
	}
	return SG_SUCCESS;
}

static SgResult sgCreatePipelineLayout(const SgApp* pApp, const SgSetLayouts* pSetLayouts, VkPipelineLayout* pPipelineLayout) {

	VkPipelineLayoutCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pSetLayouts = pSetLayouts->setLayouts,
		.setLayoutCount = pSetLayouts->setLayoutCount,
	};

	if (vkCreatePipelineLayout(pApp->device, &createInfo, VK_NULL_HANDLE, pPipelineLayout) == VK_SUCCESS) {
		sgLogInfo_Debug("[Graphics Instance]: Grapics Pipeline Layout Initialized");
	} else {
		sgLogError("[Graphics Instance]: Grapics Pipeline Layout Initialization Error");
		return -1;
	}

	return SG_SUCCESS;
}


SgResult sgCreateMaterial(const SgMaterialMap* pMaterialMap, const SgMaterialCreateInfo* pCreateInfo, SgMaterial* pMaterial) {
	SgGraphicsPipelineBuilder graphicsPipelineBuilder;
	sgFillGraphicsPipelineBuilder(pMaterialMap->pApp, &graphicsPipelineBuilder);
	sgSetGraphicsPipelineBuilderStages(pCreateInfo, &graphicsPipelineBuilder);
	sgCreateDescriptorSetLayout(pMaterialMap->pApp, pCreateInfo, &pMaterial->setLayouts);
	sgCreatePipelineLayout(pMaterialMap->pApp, &pMaterial->setLayouts, &graphicsPipelineBuilder.pipelineLayout);
	sgBuildGraphicsPipeline(pMaterialMap->pApp, &graphicsPipelineBuilder, pMaterialMap->renderPass, &pMaterial->pipeline);
	free(graphicsPipelineBuilder.pShaderStages);
	pMaterial->pName = pCreateInfo->pMaterialName;
	pMaterial->ppShaders = pCreateInfo->ppShaders;
	pMaterial->shaderCount = pCreateInfo->shaderCount;
	pMaterial->pipelineLayout = graphicsPipelineBuilder.pipelineLayout;

	pMaterial->resourceSetBindingCount = pCreateInfo->resourceSetBindingCount;
	pMaterial->pResourceBindingCount = pCreateInfo->pResourceBindingCount;
	pMaterial->ppResourceBindings = pCreateInfo->ppResourceBindings;
	pMaterial->descriptorSetCount = pCreateInfo->resourceSetBindingCount;
	SG_CALLOC_NUM(pMaterial->pDescriptorSets, pMaterial->descriptorSetCount);

	return SG_SUCCESS;
}

SgMaterial* sgAddMaterial(const SgMaterialCreateInfo* pCreateInfo, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	
	SgMaterial material = {
		.pName = pCreateInfo->pMaterialName,
	};

	SgMaterial* pMaterial = hashmap_get(pMaterialMap->pMaterialMap, &material);
	if (pMaterial) {
		return pMaterial;
	} else {
		sgCreateMaterial(pMaterialMap, pCreateInfo, &material);
		hashmap_set(pMaterialMap->pMaterialMap, &material);
	}
	*ppMaterialMap = pMaterialMap;
	return pMaterial;
}


static _Bool materialDescriptorPoolCountGetIter(const void *item, void *udata) {
    const SgMaterial *pMaterial = item;
	if (strlen(pMaterial->pName) == 0) {
		return 0;
	}
	uint32_t* poolSizeCount = udata;
	for(uint32_t i = 0; i < pMaterial->resourceSetBindingCount; ++i) {
		*poolSizeCount += pMaterial->pResourceBindingCount[i];
	}

    return 1;
}

struct SgFillerDescriptorPoolSizes {
	VkDescriptorPoolSize* pPoolSizes;
	uint32_t offset;
};

static _Bool materialDescriptorPoolFillIter(const void *item, void *udata) {
    const SgMaterial *pMaterial = item;
	if (strlen(pMaterial->pName) == 0) {
		return 0;
	}
	struct SgFillerDescriptorPoolSizes* pPoolSizes = udata;

	for (uint32_t i = 0; i < pMaterial->resourceSetBindingCount; ++i) {
		for (uint32_t j = 0; j < pMaterial->pResourceBindingCount[i]; ++j) {

			switch (pMaterial->ppResourceBindings[i][j].type) {
			case SG_RESOURCE_TYPE_TEXTURE_2D:
				pPoolSizes->pPoolSizes[pPoolSizes->offset].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
			case SG_RESOURCE_TYPE_MESH:
				pPoolSizes->pPoolSizes[pPoolSizes->offset].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case SG_RESOURCE_TYPE_UNIFORM:
				pPoolSizes->pPoolSizes[pPoolSizes->offset].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				break;
			}
			pPoolSizes->pPoolSizes[pPoolSizes->offset].descriptorCount = 1;
			++pPoolSizes->offset;
		}
	}
    return 1;
}

static _Bool materialDescriptorSetCountGetIter(const void *item, void *udata) {
    const SgMaterial *pMaterial = item;
	if (strlen(pMaterial->pName) == 0) {
		return 0;
	}
	uint32_t* setCount = udata;
	*setCount += pMaterial->resourceSetBindingCount;

    return 1;
}

struct SgDescriptorAllocInfo {
	SgApp*            pApp;
	VkDescriptorPool* pDescriptorPool;
};

static _Bool materialDescriptorSetAllocateIter(const void *item, void *udata) {
    const SgMaterial *pMaterial = item;
	if (strlen(pMaterial->pName) == 0) {
		return 0;
	}
	struct SgDescriptorAllocInfo* pPool = udata;
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .descriptorPool = *pPool->pDescriptorPool,
	    .descriptorSetCount = pMaterial->descriptorSetCount,
		.pSetLayouts = pMaterial->setLayouts.setLayouts,
	};

	vkAllocateDescriptorSets(pPool->pApp->device, &descriptorSetAllocInfo, pMaterial->pDescriptorSets);

    return 1;
}


SgResult sgInitMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	uint32_t poolSizeCount = 0;
	hashmap_scan(pMaterialMap->pMaterialMap, materialDescriptorPoolCountGetIter, &poolSizeCount);
	VkDescriptorPoolSize* pPoolSizes;
	SG_CALLOC_NUM(pPoolSizes, poolSizeCount);
	struct SgFillerDescriptorPoolSizes poolSizes = {
		.pPoolSizes = pPoolSizes,
		.offset = 0,
	};

	uint32_t setCount = 0;
	hashmap_scan(pMaterialMap->pMaterialMap, materialDescriptorSetCountGetIter, &setCount);
	hashmap_scan(pMaterialMap->pMaterialMap, materialDescriptorPoolFillIter, &poolSizes);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	    .pPoolSizes = pPoolSizes,
	    .poolSizeCount = poolSizeCount,
	    .maxSets = setCount,
	};

	vkCreateDescriptorPool(pApp->device, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &pMaterialMap->descriptorPool);
	struct SgDescriptorAllocInfo info = {
		.pApp = pApp,
		.pDescriptorPool = &pMaterialMap->descriptorPool,
	};
	// Allocate Descriptor Sets
	hashmap_scan(pMaterialMap->pMaterialMap, materialDescriptorSetAllocateIter, &info);
	*ppMaterialMap=pMaterialMap;
	return SG_SUCCESS;
}

SgMaterial* sgGetMaterial(const char* pMaterialName, const SgMaterialMap* pMaterialMap) {
	SgMaterial material = {
		.pName = pMaterialName,
	};
	SgMaterial* pMaterial = hashmap_get(pMaterialMap->pMaterialMap, &material);
	return pMaterial;
}

SgResult sgAddMaterialRenderObjects(const SgRenderObjectCreateInfo* pCreateInfo, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	SgMaterialRenderObjects renderObject = {
		.pRenderObjects = pCreateInfo->pRenderObjects,
		.renderObjectCount = pCreateInfo->renderObjectCount,
		.materialObjectsName = pCreateInfo->materialObjectsName,
		.materialName = pCreateInfo->materialName,
		.ppResources = pCreateInfo->ppResources,
		.pResourceSetBindings = pCreateInfo->pResourceSetBindings,
		.resourceCount = pCreateInfo->resourceCount,
		.resourceSetCount = pCreateInfo->resourceSetCount,
	};
	SG_CALLOC_NUM(renderObject.pWriteDescriptorSets, renderObject.resourceSetCount);
	SgMaterialRenderObjects* pMaterialRenderObject = hashmap_get(pMaterialMap->pMaterialRenderObjectMap, &renderObject);
	if (pMaterialRenderObject) {
		return SG_SUCCESS;
	}
	hashmap_set(pMaterialMap->pMaterialRenderObjectMap, &renderObject);
	return SG_SUCCESS;
}

_Bool materialRenderObjectWrite(const void *item, void *udata) {
	const SgMaterialRenderObjects* pRenderObject = item;
	SgMaterialMap* pMMap = udata;
	struct hashmap* pMaterialMap = pMMap->pMaterialMap;
	SgMaterial findMaterial = {
		.pName = pRenderObject->materialName,
	};
	SgMaterial* pMaterial = hashmap_get(pMaterialMap, &findMaterial);
	if (pMaterial == NULL) {
		sgLogDebug("Material not found");
	}

	for (uint32_t i = 0; i < pRenderObject->resourceSetCount; ++i) {
		uint32_t resCount = 0;
		for (uint32_t j = 0; j < pRenderObject->resourceCount; ++j) {
			if (i != pRenderObject->pResourceSetBindings[j]) {
				continue;
			}

			pRenderObject->pWriteDescriptorSets[resCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

			pRenderObject->pWriteDescriptorSets[resCount].dstSet = pMaterial->pDescriptorSets[i];
			pRenderObject->pWriteDescriptorSets[resCount].dstBinding = pRenderObject->ppResources[j]->resourceBinding.binding;
			pRenderObject->pWriteDescriptorSets[resCount].descriptorCount = 1;

			VkDescriptorImageInfo imageInfo;
			VkDescriptorBufferInfo bufferInfo;
			if (pRenderObject->ppResources[j]->resourceBinding.type == SG_RESOURCE_TYPE_TEXTURE_2D) {
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = pRenderObject->ppResources[j]->imageView;
				imageInfo.sampler   = pRenderObject->ppResources[j]->imageSampler;
			} else {
				bufferInfo.buffer = pRenderObject->ppResources[j]->dataBuffer.buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = pRenderObject->ppResources[j]->dataBuffer.size;
			}
			pRenderObject->pWriteDescriptorSets[resCount].pImageInfo = &imageInfo;
			pRenderObject->pWriteDescriptorSets[resCount].pBufferInfo = &bufferInfo;

			switch (pRenderObject->ppResources[j]->resourceBinding.type) {
				case (SG_RESOURCE_TYPE_UNIFORM):
					pRenderObject->pWriteDescriptorSets[resCount].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_MESH):
					pRenderObject->pWriteDescriptorSets[resCount].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_TEXTURE_2D):
					pRenderObject->pWriteDescriptorSets[resCount].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
			}
			++resCount;
		}
		vkUpdateDescriptorSets(pMMap->pApp->device, resCount, pRenderObject->pWriteDescriptorSets, 0, VK_NULL_HANDLE);
	}
	return 1;
}

SgResult sgWriteMaterialRenderObjects(SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	hashmap_scan(pMaterialMap->pMaterialRenderObjectMap, materialRenderObjectWrite, pMaterialMap);

	return SG_SUCCESS;
}
