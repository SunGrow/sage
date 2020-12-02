#include "sg_rend.h"
#include "sage_base.h"
#ifdef _DEBUG
#include "debug/sg_rend_debug.h"
#endif
#include "device/sg_rend_device.h"
#include "surface/sg_rend_surface.h"
#include "allocator/sg_rend_allocator.h"
#include "command/sg_rend_command.h"
#include "resource/sg_resource.h"
#include <stdlib.h>
#include "log.h"

static const char* SG_SURF_EXT[] = {
	VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _DEBUG
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
};


SgResult createGLFWwindow(const SgAppCreateInfo *pCreateInfo, SgApp *pApp) {
	if (pCreateInfo->flags & SG_APP_WINDOW_RESIZABLE) {
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (pCreateInfo->flags & SG_APP_WINDOW_FULLSCREEN) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		 
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		pApp->pWindow = glfwCreateWindow(mode->width, mode->height, pCreateInfo->pName, monitor, NULL);
	} else {
		pApp->pWindow = glfwCreateWindow(pCreateInfo->size[0], pCreateInfo->size[1], pCreateInfo->pName, NULL, NULL);
	}
	if(pApp->pWindow) {
		log_info("[AppInit]: GLFW window created");
	} else {
		log_fatal("[AppInit]: GLFW window creation error");
	}

	if (pCreateInfo->flags & SG_APP_CURSOR_HIDDEN) {
		glfwSetInputMode(pApp->pWindow , GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	return SG_SUCCESS;
}

SgResult createVkInstance(const SgAppCreateInfo *pCreateInfo, SgApp *pApp) {
	/* Instance */
    uint32_t apiver = volkGetInstanceVersion();
    if (apiver >= VK_API_VERSION_1_2) {
		log_info("[AppInit]: Vulkan API version 1.2 of newer found");
	} else {
		log_error("[AppInit]: Vulkan API version is too old");
	}
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = pCreateInfo->pName,
        .pEngineName = "svkr",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_2,
    };

    uint32_t glfwExtCount = 0;
    const char **glfwExt;
    glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    uint32_t extCount = NUMOF(SG_SURF_EXT) + glfwExtCount;
    const char **pExt = malloc(extCount * sizeof(*pExt));
    for (uint32_t i = 0; i < glfwExtCount; ++i) {
        pExt[i] = glfwExt[i];
    }
    for (uint32_t i = glfwExtCount; i < extCount; ++i) {
        pExt[i] = SG_SURF_EXT[i - glfwExtCount];
    }

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = VK_NULL_HANDLE,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .ppEnabledExtensionNames = pExt,
        .enabledExtensionCount = extCount,
    };
#ifdef _DEBUG
    const char *pdeblayers[] = {
        "VK_LAYER_KHRONOS_validation",
    };
    createInfo.ppEnabledLayerNames = pdeblayers;
    createInfo.enabledLayerCount = NUMOF(pdeblayers);
#endif
    VkResult vkRes = vkCreateInstance(&createInfo, VK_NULL_HANDLE, &pApp->instance);
	if (vkRes == VK_SUCCESS) {
		log_info("[AppInit]: Vulkan Instance created");
	} else { 
		log_fatal("[AppInit]: Vulkan Instance creation failure");
	}

	volkLoadInstance(pApp->instance);

    free(pExt);
	return SG_SUCCESS;
}

SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp **ppSgApp) {
	/* TODO: Implement custom allocation callbacks */
	// static could be an alternative... But it is not thread safe. Mb will change to static later.
	/* TODO: Make execution order explicit where it matters.           *
	 * Having to setup individual small createinfo structures may help */
	SgApp* pApp = calloc(1,sizeof(pApp[0]));

	glfwInit();
	volkInitialize();

	/* Window */
	createGLFWwindow(pCreateInfo, pApp);
	/* Vulkan Instance */
	createVkInstance(pCreateInfo, pApp);
#ifdef _DEBUG
	/* Vulkan Debug Callback */
	registerDebugCallback(pApp);
#endif
	/* Vulkan Surface */
	createWindowSurface(pApp);
	/* Vulkan Physical Device */
	getPhysicalDevice(pApp);
	/* Vulkan Logical Device */
	getLogicalDevice(pApp);
	vkGetDeviceQueue(pApp->device, pApp->graphicsQueueFamilyIdx, 0, &pApp->graphicsQueue);

	/* Create Vulkan Allocator */
	createAllocator(pApp);

	/* Vulkan Surface Attirbutes */
	getSurfaceAttributes(pApp);

	/* Command Pool Initialization */
	initCommandPools(pApp);

	/* End */
	*ppSgApp = pApp;

	return SG_SUCCESS;
}

SgResult sgCreateResourceSet(const SgApp* pApp, const SgResourceSetCreateInfo *pCreateInfo, SgResourceSet **ppSgResourceSet) {
	SgResourceSet* pResourceSet = calloc(1, sizeof(pResourceSet[0]));

	VkDescriptorSetLayoutBinding* pSetLayoutBindings = calloc(pCreateInfo->resourceCount, sizeof(pSetLayoutBindings[0]));

	for (uint32_t i = 0; i < pCreateInfo->resourceCount; ++i) {
		pSetLayoutBindings[i].binding = pCreateInfo->ppResources[i][0].binding;
		pSetLayoutBindings[i].descriptorCount = 1;
		pSetLayoutBindings[i].stageFlags = pCreateInfo->ppResources[i][0].stage;
		switch (pCreateInfo->ppResources[i][0].type) {
			case (SG_RESOURCE_TYPE_INDICES):
				break;
			case (SG_RESOURCE_TYPE_MESH):
				pSetLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case (SG_RESOURCE_TYPE_DYNAMIC):
				pSetLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				break;
			case (SG_RESOURCE_TYPE_TEXTURE_2D):
				pSetLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				break;
		}
	}
	pResourceSet->pSetLayoutBindings = pSetLayoutBindings;
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pBindings = pSetLayoutBindings,
		.bindingCount = pCreateInfo->resourceCount,
	};
	if(vkCreateDescriptorSetLayout(pApp->device, &setLayoutCreateInfo, VK_NULL_HANDLE, &pResourceSet->setLayout) == VK_SUCCESS) {
		log_info("[Set]: Descriptor Set Layout Init Successfull");
	} else {
		log_error("[Set]: Descriptor Set Layout Init Error");
	}
	pResourceSet->setIndex = pCreateInfo->setIndex;
	pResourceSet->resourceCount = pCreateInfo->resourceCount;
	pResourceSet->pResources = *pCreateInfo->ppResources;

	*ppSgResourceSet = pResourceSet;

	return SG_SUCCESS;
}

SgResult sgInitResourceSet(const SgApp *pApp, const SgResourceSetInitInfo *pInitInfo) {
	VkWriteDescriptorSet* pWriteDescriptorSets = calloc(pInitInfo->resourceCount, sizeof(pWriteDescriptorSets[0]));
	for (uint32_t i = 0; i < pInitInfo->resourceCount; ++i) {
		// Not sure if it works correctly. May cause issues
		pInitInfo->pResourceSet->pResources[pInitInfo->ppResources[i]->binding] = *pInitInfo->ppResources[i];
		//
		VkDescriptorImageInfo *pImageInfo = VK_NULL_HANDLE;
		VkDescriptorBufferInfo *pBufferInfo = VK_NULL_HANDLE;
		pWriteDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pWriteDescriptorSets[i].dstSet = pInitInfo->pGraphicsInstance->pDescriptorSets[pInitInfo->pResourceSet->setIndex];
		pWriteDescriptorSets[i].dstBinding = pInitInfo->ppResources[i]->binding;
		pWriteDescriptorSets[i].descriptorCount = 1;
		if (pInitInfo->ppResources[i]->type == SG_RESOURCE_TYPE_TEXTURE_2D) {
			/* Won't work unless image resource creation is completed */
			pImageInfo = malloc(sizeof(pImageInfo[0]));
			pImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			pImageInfo->imageView = pInitInfo->ppResources[i]->imageView;
			pImageInfo->sampler   = pInitInfo->ppResources[i]->imageSampler;
			pWriteDescriptorSets[i].pImageInfo = pImageInfo;

		} else {
			pBufferInfo = malloc(sizeof(pBufferInfo[0]));
			pBufferInfo->buffer = pInitInfo->ppResources[i]->dataBuffer.buffer;
			pBufferInfo->offset = 0;
			pBufferInfo->range = pInitInfo->ppResources[i]->dataBuffer.size;
			pWriteDescriptorSets[i].pBufferInfo = pBufferInfo;
		}
		switch (pInitInfo->ppResources[i]->type) {
			case (SG_RESOURCE_TYPE_DYNAMIC):
				pWriteDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				break;
			case (SG_RESOURCE_TYPE_MESH):
				pWriteDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				break;
			case (SG_RESOURCE_TYPE_TEXTURE_2D):
				pWriteDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				break;
		}
	}
	vkUpdateDescriptorSets(pApp->device, pInitInfo->resourceCount, pWriteDescriptorSets, 0, VK_NULL_HANDLE);
	return SG_SUCCESS;
}

SgResult sgCreateShader(const SgApp *pApp, const SgShaderCreateInfo* pCreateInfo, SgShader **ppShader) {
	SgShader* pShader = calloc(1, sizeof(pShader[0]));
	SgFile* pFile = pCreateInfo->pFile;
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = pFile->size,
		.pCode = pFile->pBytes,
	};
	VkResult res = vkCreateShaderModule(pApp->device, &createInfo, VK_NULL_HANDLE, &pShader->shader);

	if(res == VK_SUCCESS) {
		log_info("[Shader]: Shader Module created successfully");
	} else {
		log_error("[Shader]: Shader Module creation failure");
	}
	pShader->stage = pCreateInfo->stage;
	*ppShader = pShader;
	return SG_SUCCESS;
}

// TODO: Fix this static crap. Is a problem with an API, really. But I have no time to fix it
SgResult createRenderPass(VkDevice device, VkFormat format, VkRenderPass* pRenderPass) {
	VkAttachmentDescription attachments[] = {
	    {
	        .format = format,
	        .samples = VK_SAMPLE_COUNT_1_BIT,
	        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
	        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	    },
	    {
	        .format = VK_FORMAT_D32_SFLOAT_S8_UINT,
	        .samples = VK_SAMPLE_COUNT_1_BIT,
	        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	    },
	};

	VkAttachmentReference colorAttachment = {
	    .attachment = 0,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference depthAttachment = {
	    .attachment = 1,
	    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
	    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
	    .colorAttachmentCount = 1,
	    .pColorAttachments = &colorAttachment,
	    .pDepthStencilAttachment = &depthAttachment,
	};

	VkRenderPassCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	};
	createInfo.attachmentCount = NUMOF(attachments);
	createInfo.pAttachments = attachments;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;

	if(vkCreateRenderPass(device, &createInfo, 0, pRenderPass) == VK_SUCCESS) {
		log_info("[Graphics Instance]: Render Pass Created");
	} else {
		log_error("[Graphics Instance]: Render Pass Creation Failure");
	}

	return SG_SUCCESS;
}

SgResult createVkSwapchain(const SgApp* pApp, VkSwapchainKHR oldswapchain, VkSwapchainKHR* pSwapchain) {
	VkSurfaceCapabilitiesKHR surfcap = pApp->surfaceAttributes.surfaceCapabilities;
	VkCompositeAlphaFlagBitsKHR surfacecomposite =
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
	    .compositeAlpha = surfacecomposite,
	    .presentMode = pApp->surfaceAttributes.presentMode,
	    .oldSwapchain = oldswapchain,
	};
	if (vkCreateSwapchainKHR(pApp->device, &createinfo, VK_NULL_HANDLE, pSwapchain) == VK_SUCCESS) {
	} else {
		log_warn("[Graphics Instance]: Swapchain Creation Failure");
		return 1;
	}
	return SG_SUCCESS;
}

SgResult sgCreateSwapchain(const SgApp *pApp, SgSwapchainCreateInfo *pCreateInfo, SgSwapchain *pSwapchain) {

	createVkSwapchain(pApp, pCreateInfo->oldSwapchain, &pSwapchain->swapchain);
	pSwapchain->extent = pApp->surfaceAttributes.surfaceCapabilities.currentExtent;

	/* Frame Image Creation */
	vkGetSwapchainImagesKHR(pApp->device, pSwapchain->swapchain, &pSwapchain->imageCount, VK_NULL_HANDLE);
	pSwapchain->pFrameImages = malloc(sizeof(VkImage) * pSwapchain->imageCount);
	vkGetSwapchainImagesKHR(pApp->device, pSwapchain->swapchain, &pSwapchain->imageCount, pSwapchain->pFrameImages);

	pSwapchain->pFrameImageViews = malloc(sizeof(pSwapchain->pFrameImageViews[0]) * pSwapchain->imageCount);

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
	};
	sgCreateImage(pApp, &depthImageCreateInfo, &pSwapchain->depthImage);

	SgImageViewCreateInfo createInfo = {
		.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT,
		.pImage = &pSwapchain->depthImage,
		.type = VK_IMAGE_VIEW_TYPE_2D,
	};
	sgCreateImageView(pApp, &createInfo, &pSwapchain->depthImageView);

	/* FrameBuffer */
	VkFramebufferCreateInfo framebufferCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
	    .renderPass = pCreateInfo->renderPass,
	    .width = pSwapchain->extent.width,
	    .height = pSwapchain->extent.height,
	    .layers = 1,
	};
	pSwapchain->pFrameBuffers = malloc(sizeof(pSwapchain->pFrameBuffers[0]) * pSwapchain->imageCount);


	for (uint32_t i = 0; i < pSwapchain->imageCount; ++i) {
		VkImageView pAttachments[] = {pSwapchain->pFrameImageViews[i], pSwapchain->depthImageView.imageView};
		framebufferCreateInfo.attachmentCount = NUMOF(pAttachments);
		framebufferCreateInfo.pAttachments = pAttachments;
		if (vkCreateFramebuffer(pApp->device, &framebufferCreateInfo, VK_NULL_HANDLE, &pSwapchain->pFrameBuffers[i]) == VK_SUCCESS) {
		} else {
			log_error("[Graphics Instance]: Framebuffer %d Creation Failure", i);
		}
	}
	log_info("[Graphics Instance]: Framebuffer Creation Finished");
	return SG_SUCCESS;
}

SgResult sgCreateGraphicsInstance(const SgApp *pApp, const SgGraphicsInstanceCreateInfo *pCreateInfo, SgGraphicsInstance **ppGraphicsInstance) {
	SgGraphicsInstance* pGraphicsInstance = calloc(1, sizeof(pGraphicsInstance[0]));
	pGraphicsInstance->ppSets = pCreateInfo->ppSets;
	/* Create Render Passes */
	createRenderPass(pApp->device, pApp->surfaceAttributes.format.format, &pGraphicsInstance->renderPass);
	
	/* Create Pipeline Layout */
	pGraphicsInstance->pDescriptorSetLayouts =
	    malloc(sizeof(pGraphicsInstance->pDescriptorSetLayouts[0]) * pCreateInfo->setCount);
	for (uint32_t i = 0; i < pCreateInfo->setCount; ++i) {
		pGraphicsInstance->pDescriptorSetLayouts[i] = pCreateInfo->ppSets[i][0].setLayout;
	}
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .setLayoutCount = pCreateInfo->setCount,
	    .pSetLayouts = pGraphicsInstance->pDescriptorSetLayouts,
	};
	if(vkCreatePipelineLayout(pApp->device, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &pGraphicsInstance->pipelineLayout) == VK_SUCCESS) {
		log_info("[Graphics Instance]: Grapics Pipeline Layout Initialized");
	} else {
		log_error("[Graphics Instance]: Grapics Pipeline Layout Initialization Error");
	}
	pGraphicsInstance->setCount = pCreateInfo->setCount;
	/* Create Descriptor Sets */
	uint32_t poolSizeCount = 0;
	for (uint32_t i = 0; i < pCreateInfo->setCount; ++i) {
		for (uint32_t j = 0; j < pCreateInfo->ppSets[i]->resourceCount; ++j) {
			++poolSizeCount;
		}
	}
	VkDescriptorPoolSize *pPoolSizes = malloc(poolSizeCount * sizeof(pPoolSizes[0]));
	for (uint32_t i = 0; i < pCreateInfo->setCount; ++i) {
		for (uint32_t j = 0; j < pCreateInfo->ppSets[i]->resourceCount; ++j) {
			switch (pCreateInfo->ppSets[i]->pResources[j].type) {
				case SG_RESOURCE_TYPE_TEXTURE_2D:
					pPoolSizes[i + j * pCreateInfo->setCount].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
				case SG_RESOURCE_TYPE_MESH:
					pPoolSizes[i + j * pCreateInfo->setCount].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case SG_RESOURCE_TYPE_DYNAMIC:
					pPoolSizes[i + j * pCreateInfo->setCount].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					break;
			}
			pPoolSizes[i + j * pCreateInfo->setCount].descriptorCount = 1;
		}
	}
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	    .pPoolSizes = pPoolSizes,
	    .poolSizeCount = poolSizeCount,
	    .maxSets = pCreateInfo->setCount,
	};
	vkCreateDescriptorPool(pApp->device, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &pGraphicsInstance->descriptorPool);
	free(pPoolSizes);
	pGraphicsInstance->pDescriptorSets = malloc(pGraphicsInstance->setCount * sizeof(pGraphicsInstance->pDescriptorSets[0]));
	VkDescriptorSetLayout *pDescriptorSetLayouts = malloc(sizeof(pDescriptorSetLayouts[0]) * pGraphicsInstance->setCount);
	for (uint32_t i = 0; i < pGraphicsInstance->setCount; ++i) {
		pDescriptorSetLayouts[i] = pGraphicsInstance->ppSets[i]->setLayout;
	}
	VkDescriptorSetAllocateInfo descrriptorSetAllocInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .descriptorPool = pGraphicsInstance->descriptorPool,
	    .descriptorSetCount = pGraphicsInstance->setCount,
	    .pSetLayouts = pDescriptorSetLayouts,
	};
	vkAllocateDescriptorSets(pApp->device, &descrriptorSetAllocInfo, pGraphicsInstance->pDescriptorSets);
	free(pDescriptorSetLayouts);

	/* Create Swapchain */
	SgSwapchainCreateInfo swapchainCreateInfo = {
		.renderPass = pGraphicsInstance->renderPass,
	};
	sgCreateSwapchain(pApp, &swapchainCreateInfo, &pGraphicsInstance->swapchain);

	/* Create Graphics Pipeline */
	VkPipelineShaderStageCreateInfo *pShaderStageCreateInfos = calloc(pCreateInfo->shaderCount, sizeof(pShaderStageCreateInfos[0]));
	for (uint32_t i = 0; i < pCreateInfo->shaderCount; ++i) {
		pShaderStageCreateInfos[i] = (VkPipelineShaderStageCreateInfo) {
	        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
	        .stage = pCreateInfo->ppShaders[i]->stage,
	        .module = pCreateInfo->ppShaders[i]->shader,
	        .pName = "main",
		};
	}

	VkPipelineVertexInputStateCreateInfo vertexinput = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo inputassembly = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineViewportStateCreateInfo viewportstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .scissorCount = 1,
	};
	VkPipelineMultisampleStateCreateInfo multisamplestate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	};

	VkPipelineDepthStencilStateCreateInfo depthstencilstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable = VK_TRUE,
	    .depthWriteEnable = VK_TRUE,
	    .depthCompareOp = VK_COMPARE_OP_LESS,
	};

	VkPipelineColorBlendAttachmentState colorAttachmentState = {
	    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	    	.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
	    	.dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
	    	.colorBlendOp = VK_BLEND_OP_ADD,
	    	.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
	    	.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
	    	.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	    .attachmentCount = 1,
	    .pAttachments = &colorAttachmentState,
	};

	VkDynamicState dynamicstates[] = {VK_DYNAMIC_STATE_VIEWPORT,
	                                  VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo dynamicstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
	    .dynamicStateCount = NUMOF(dynamicstates),
	    .pDynamicStates = dynamicstates,
	};
	VkPipelineRasterizationStateCreateInfo rasterizationState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	    .lineWidth = 1.f,
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
	    .stageCount = pCreateInfo->shaderCount,
	    .pStages = pShaderStageCreateInfos,
	    .pVertexInputState = &vertexinput,
	    .pInputAssemblyState = &inputassembly,
	    .pViewportState = &viewportstate,
	    .pRasterizationState = &rasterizationState,
	    .pMultisampleState = &multisamplestate,
	    .pDepthStencilState = &depthstencilstate,
	    .pColorBlendState = &colorBlendState,
	    .pDynamicState = &dynamicstate,
	    .layout = pGraphicsInstance->pipelineLayout,
	    .renderPass = pGraphicsInstance->renderPass,
	};

	vkCreateGraphicsPipelines(pApp->device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, 0, &pGraphicsInstance->graphicsPipeline);
	free(pShaderStageCreateInfos);

	*ppGraphicsInstance = pGraphicsInstance;
	return SG_SUCCESS;
}

SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo) {
	SgApp *pApp = pUpdateInfo->pApp;
	SgGraphicsInstance *pGraphicsInstance = pUpdateInfo->pGraphicsInstance;
	if(glfwWindowShouldClose(pApp->pWindow))
		return 0;
	glfwPollEvents();
	vkAcquireNextImageKHR();


	return 1;
}

void sgDestroyApp(SgApp **ppApp) {
	SgApp* pApp = *ppApp;
	vkDeviceWaitIdle(pApp->device);
	vkDestroyDevice(pApp->device, VK_NULL_HANDLE);
}
