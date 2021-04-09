#include "sg_material.h"
#include <stdlib.h>
#include <string.h>

static int materialRenderObjectsKeyCompare(const void* a, const void* b, void* udata) {
	const SgMaterialRenderObjects* keyA = a;
	const SgMaterialRenderObjects* keyB = b;
	return strcmp(keyA->pName, keyB->pName);
}

static uint64_t materialRenderObjectsKeyHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SgMaterialRenderObjects* key = item;
    return hashmap_sip(key->pName, strlen(key->pName), seed0, seed1);
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
	// max binding+1
	uint32_t descriptorSetCount = 0;
	for (uint32_t i = 0; i < pCreateInfo->resourceBindingCount; ++i) {
		if (descriptorSetCount < pCreateInfo->pResourceBindings[i].setBinding) {
			descriptorSetCount = pCreateInfo->pResourceBindings[i].setBinding;
		}
	}
	++descriptorSetCount;
	//
	uint32_t* pSetResourceCount; // count of resouces on each set
	SG_CALLOC_NUM(pSetResourceCount, descriptorSetCount);
	for (uint32_t i = 0; i < pCreateInfo->resourceBindingCount; ++i) {
		++pSetResourceCount[pCreateInfo->pResourceBindings[i].setBinding];
	}
	VkDescriptorSetLayoutBinding** ppSetLayoutBindings;
	SG_CALLOC_NUM(ppSetLayoutBindings, descriptorSetCount);
	SG_CALLOC_NUM(pSetLayouts->pSetLayouts, descriptorSetCount);
	pSetLayouts->setLayoutCount = descriptorSetCount;

	uint32_t resourceCount = 0;
	for (uint32_t i = 0; i < descriptorSetCount; ++i) {
		SG_CALLOC_NUM(ppSetLayoutBindings[i], pSetResourceCount[i]);
		for (uint32_t j = 0; j < pSetResourceCount[i]; ++j) {
			ppSetLayoutBindings[i][j].binding    = pCreateInfo->pResourceBindings[resourceCount].binding;
			ppSetLayoutBindings[i][j].stageFlags = pCreateInfo->pResourceBindings[resourceCount].stage;
			ppSetLayoutBindings[i][j].descriptorCount = 1;
			switch (pCreateInfo->pResourceBindings[resourceCount].type) {
				case (SG_RESOURCE_TYPE_INDICES):
					break;
				case (SG_RESOURCE_TYPE_MESH):
					ppSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_UNIFORM):
					ppSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_TEXTURE_2D):
					ppSetLayoutBindings[i][j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
			}
			++resourceCount;
		}
		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.pBindings = ppSetLayoutBindings[i],
			.bindingCount = pSetResourceCount[i],
		};
		VkResult result = vkCreateDescriptorSetLayout(pApp->device, &setLayoutCreateInfo, VK_NULL_HANDLE, &pSetLayouts->pSetLayouts[i]);
		if(result == VK_SUCCESS) {
			sgLogInfo_Debug("[Set]: Descriptor Set Layout Init Successfull");
		} else {
			sgLogError("[Set]: Descriptor Set Layout Init Error");
		}
		free(ppSetLayoutBindings[i]);
	}
	free(ppSetLayoutBindings);
	free(pSetResourceCount);
	return SG_SUCCESS;
}

static SgResult sgCreatePipelineLayout(const SgApp* pApp, const SgSetLayouts* pSetLayouts, VkPipelineLayout* pPipelineLayout) {

	VkPipelineLayoutCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pSetLayouts = pSetLayouts->pSetLayouts,
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
	if (graphicsPipelineBuilder.pShaderStages) {
		free(graphicsPipelineBuilder.pShaderStages);
	}
	pMaterial->pName = pCreateInfo->pMaterialName;
	pMaterial->ppShaders = pCreateInfo->ppShaders;
	pMaterial->shaderCount = pCreateInfo->shaderCount;
	pMaterial->pipelineLayout = graphicsPipelineBuilder.pipelineLayout;
	pMaterial->renderObjectCount = pCreateInfo->renderObjectCount;

	pMaterial->resourceBindingCount = pCreateInfo->resourceBindingCount;
	pMaterial->pResourceBindings = pCreateInfo->pResourceBindings;
	// max binding+1
	uint32_t descriptorSetLayoutCount = 0;
	for (uint32_t i = 0; i < pCreateInfo->resourceBindingCount; ++i) {
		if (descriptorSetLayoutCount < pCreateInfo->pResourceBindings[i].setBinding) {
			descriptorSetLayoutCount = pCreateInfo->pResourceBindings[i].setBinding;
		}
	}
	++descriptorSetLayoutCount;
	pMaterial->descriptorSetCount = descriptorSetLayoutCount;
	///

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
	*poolSizeCount+= pMaterial->resourceBindingCount;

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

	for (uint32_t i = 0; i < pMaterial->resourceBindingCount; ++i) {
		switch (pMaterial->pResourceBindings[i].type) {
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
		pPoolSizes->pPoolSizes[pPoolSizes->offset].descriptorCount = pMaterial->renderObjectCount;
		++pPoolSizes->offset;
	}
    return 1;
}

static _Bool materialDescriptorSetCountGetIter(const void *item, void *udata) {
    const SgMaterial *pMaterial = item;
	if (strlen(pMaterial->pName) == 0) {
		return 0;
	}
	uint32_t* setCount = udata;
	*setCount += pMaterial->resourceBindingCount;

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
		.ppResources = pCreateInfo->ppResources,
		.resourceCount = pCreateInfo->resourceCount,

		.pName = pCreateInfo->pName,
		.materialName = pCreateInfo->materialName,
	};
	//
	SgMaterial findMaterial = {
		.pName = pCreateInfo->materialName,
	};
	SgMaterial* pMaterial = hashmap_get(pMaterialMap->pMaterialMap, &findMaterial);
	// max binding+1
	uint32_t descriptorSetCount = 0;
	for (uint32_t i = 0; i < pMaterial->resourceBindingCount; ++i) {
		if (descriptorSetCount < pMaterial->pResourceBindings[i].setBinding) {
			descriptorSetCount = pMaterial->pResourceBindings[i].setBinding;
		}
	}
	//
	++descriptorSetCount;
	//
	renderObject.descriptorSetCount = descriptorSetCount;

	SG_CALLOC_NUM(renderObject.pWriteDescriptorSets, renderObject.resourceCount);
	SG_CALLOC_NUM(renderObject.pDescriptorSets, renderObject.descriptorSetCount);
	SgMaterialRenderObjects* pMaterialRenderObject = hashmap_get(pMaterialMap->pMaterialRenderObjectMap, &renderObject);
	if (pMaterialRenderObject) {
		return SG_SUCCESS;
	}
	hashmap_set(pMaterialMap->pMaterialRenderObjectMap, &renderObject);

	// Allocate Descriptor Sets
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .descriptorPool = pMaterialMap->descriptorPool,
	    .descriptorSetCount = renderObject.descriptorSetCount,
		.pSetLayouts = pMaterial->setLayouts.pSetLayouts,
	};

	vkAllocateDescriptorSets(pMaterialMap->pApp->device, &descriptorSetAllocInfo, renderObject.pDescriptorSets);
	//
	return SG_SUCCESS;
}

void sgUpdateRenderObjects(const SgRenderObjectUpdateInfo* pUpdateInfo, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	SgMaterialRenderObjects* pMaterialRenderObject;
	SgMaterialRenderObjects findMaterialRenderObject = {
		.pName = pUpdateInfo->pName,
	};
	pMaterialRenderObject = hashmap_get(pMaterialMap->pMaterialRenderObjectMap, &findMaterialRenderObject);
	if (pMaterialRenderObject) {
		pMaterialRenderObject->pRenderObjects = pUpdateInfo->pRenderObjects;
		pMaterialRenderObject->renderObjectCount = pUpdateInfo->renderObjectCount;
	} else {
		sgLogDebug_Debug("Render Object %s for update not found", pUpdateInfo->pName);
	}
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
	// max binding+1
	uint32_t descriptorSetCount = 0;
	for (uint32_t i = 0; i < pMaterial->resourceBindingCount; ++i) {
		if (descriptorSetCount < pMaterial->pResourceBindings[i].setBinding) {
			descriptorSetCount = pMaterial->pResourceBindings[i].setBinding;
		}
	}
	++descriptorSetCount;
	//

	VkDescriptorImageInfo** ppImageInfo;
	SG_CALLOC_NUM(ppImageInfo, pRenderObject->resourceCount);
	VkDescriptorBufferInfo** ppBufferInfo;
	SG_CALLOC_NUM(ppBufferInfo, pRenderObject->resourceCount);
	for (uint32_t i = 0; i < descriptorSetCount; ++i) {
		uint32_t resCount = 0;
		for (uint32_t j = 0; j < pRenderObject->resourceCount; ++j) {
			if (i != pMaterial->pResourceBindings[j].setBinding) {
				continue;
			}

			pRenderObject->pWriteDescriptorSets[resCount].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

			pRenderObject->pWriteDescriptorSets[resCount].dstSet = pRenderObject->pDescriptorSets[i];
			pRenderObject->pWriteDescriptorSets[resCount].dstBinding = pMaterial->pResourceBindings[j].binding;
			pRenderObject->pWriteDescriptorSets[resCount].descriptorCount = 1;

			VkDescriptorImageInfo* pImageInfo;
			VkDescriptorBufferInfo* pBufferInfo;
			SG_CALLOC_NUM(pImageInfo, 1);
			SG_CALLOC_NUM(pBufferInfo, 1);
			ppImageInfo[i] = pImageInfo;
			ppBufferInfo[i] = pBufferInfo;
			if (pMaterial->pResourceBindings[j].type != pRenderObject->ppResources[j]->type) {
				sgLogDebug("id: %d type: %d does not match %d", j, pMaterial->pResourceBindings[j].type, pRenderObject->ppResources[j]->type);
			}

			if (pMaterial->pResourceBindings[j].type == SG_RESOURCE_TYPE_TEXTURE_2D) {
				pImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				pImageInfo->imageView = pRenderObject->ppResources[j]->imageView;
				pImageInfo->sampler   = pRenderObject->ppResources[j]->imageSampler;
			} else {
				pBufferInfo->buffer = pRenderObject->ppResources[j]->dataBuffer.buffer;
				pBufferInfo->offset = 0;
				pBufferInfo->range = pRenderObject->ppResources[j]->dataBuffer.size;
			}
			pRenderObject->pWriteDescriptorSets[resCount].pImageInfo = pImageInfo;
			pRenderObject->pWriteDescriptorSets[resCount].pBufferInfo = pBufferInfo;

			switch (pMaterial->pResourceBindings[j].type) {
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
	for (uint32_t i = 0; i < pRenderObject->resourceCount; ++i) {
		if (ppBufferInfo[i]) {
			free(ppBufferInfo[i]);
		}
		if (ppImageInfo[i]) {
			free(ppImageInfo[i]);
		}
	}
	free(ppImageInfo);
	free(ppBufferInfo);
	return 1;
}

SgResult sgWriteMaterialRenderObjects(SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	hashmap_scan(pMaterialMap->pMaterialRenderObjectMap, materialRenderObjectWrite, pMaterialMap);

	return SG_SUCCESS;
}

_Bool sgFreeMaterialMapIter(const void* item, void* data) {
	const SgMaterial* pMaterial = item;
	SgMaterialMap* pMaterialMap = data;
	VkDevice device = pMaterialMap->pApp->device;
	vkDestroyPipelineLayout(device, pMaterial->pipelineLayout, VK_NULL_HANDLE); // TODO: pipeline layout hashmap
	vkDestroyPipeline(device, pMaterial->pipeline, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < pMaterial->setLayouts.setLayoutCount; ++i) {
		vkDestroyDescriptorSetLayout(device, pMaterial->setLayouts.pSetLayouts[i], VK_NULL_HANDLE);
	}
	free(pMaterial->setLayouts.pSetLayouts);
	return 1;
}
_Bool sgFreeMaterialRenderObjectMapIter(const void* item, void* data) {
	const SgMaterialRenderObjects* pMaterialRenderObject = item;
	free(pMaterialRenderObject->pWriteDescriptorSets);
	free(pMaterialRenderObject->pDescriptorSets);
	return 1;
}

void sgDestroySwapchain(SgApp* pApp, SgSwapchain* pSwapchain) {
	vmaDestroyImage(pApp->allocator, pSwapchain->depthImage.image, pSwapchain->depthImage.allocation);
	vkDestroyImageView(pApp->device, pSwapchain->depthImageView.imageView, VK_NULL_HANDLE);
	vmaDestroyImage(pApp->allocator, pSwapchain->blendImage.image, pSwapchain->blendImage.allocation);
	vkDestroyImageView(pApp->device, pSwapchain->blendImageView.imageView, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < pSwapchain->imageCount; ++i) {
		vkDestroyImageView(pApp->device, pSwapchain->pFrameImageViews[i], VK_NULL_HANDLE);
		vkDestroyFramebuffer(pApp->device, pSwapchain->pFrameBuffers[i], VK_NULL_HANDLE);
	}
	vkDestroySwapchainKHR(pApp->device, pSwapchain->swapchain, VK_NULL_HANDLE);
}

void sgDestroyMaterialMap(SgApp* pApp, SgMaterialMap** ppMaterialMap) {
	SgMaterialMap* pMaterialMap = *ppMaterialMap;
	if(pMaterialMap) {
		hashmap_scan(pMaterialMap->pMaterialRenderObjectMap, sgFreeMaterialRenderObjectMapIter, pMaterialMap);
		hashmap_free(pMaterialMap->pMaterialRenderObjectMap);
		hashmap_scan(pMaterialMap->pMaterialMap, sgFreeMaterialMapIter, pMaterialMap);
		hashmap_free(pMaterialMap->pMaterialMap);
		vkDestroyRenderPass(pApp->device, pMaterialMap->renderPass, VK_NULL_HANDLE);
		sgDestroySwapchain(pApp, &pMaterialMap->swapchain);
		vkDestroyDescriptorPool(pApp->device, pMaterialMap->descriptorPool, VK_NULL_HANDLE);

		free(pMaterialMap);
	}
	*ppMaterialMap = NULL;
	return;
}
