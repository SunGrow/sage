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
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
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
	/* TODO: Actually find sample count supported on a device */
	pApp->msaaSampleCount = VK_SAMPLE_COUNT_2_BIT;
	/* Vulkan Logical Device */
	getLogicalDevice(pApp);
	vkGetDeviceQueue(pApp->device, pApp->graphicsQueueFamilyIdx, 0, &pApp->graphicsQueue);

	/* Create Vulkan Allocator */
	createAllocator(pApp);

	/* Vulkan Surface Attirbutes */
	getSurfaceAttributes(pApp);

	/* Command Pool Initialization */
	initCommandPools(pApp);

	/* Synchronization Primitive creation*/
	VkSemaphoreCreateInfo semaphoreCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT,
	};
	VkResult res = VK_SUCCESS;
	for (uint32_t i = 0; i < SG_FRAME_QUEUE_LENGTH; ++i) {
		res |= vkCreateSemaphore(pApp->device, &semaphoreCreateInfo, VK_NULL_HANDLE, &pApp->pFrameReadySemaphore[i]);
		res |= vkCreateSemaphore(pApp->device, &semaphoreCreateInfo, VK_NULL_HANDLE, &pApp->pFrameFinishedSemaphore[i]);
		res |= vkCreateFence(pApp->device, &fenceCreateInfo, VK_NULL_HANDLE, &pApp->pFrameFences[i]);
	}
	if (res) {
		log_error("[App Init]: Synchonization Primitive init failure");
	}

	/* End */
	*ppSgApp = pApp;

	return SG_SUCCESS;
}

GLFWwindow *sgGetGLFWwindow(SgApp *pApp) {
	return pApp->pWindow;
}

void sgSetKeyCallback(SgApp** pApp, GLFWkeyfun callback) {
	glfwSetKeyCallback(pApp[0]->pWindow, callback);
}
void sgSetCursorPosCallback(SgApp** pApp, GLFWcursorposfun callback) {
	glfwSetCursorPosCallback(pApp[0]->pWindow, callback);
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
			case (SG_RESOURCE_TYPE_UNIFORM):
				pSetLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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
	pResourceSet->ppResources = pCreateInfo->ppResources;

	*ppSgResourceSet = pResourceSet;

	return SG_SUCCESS;
}

SgResult sgInitResourceSet(const SgApp *pApp, SgResourceSetInitInfo *pInitInfo, SgResourceSet** ppResourceSet) {
	SgResourceSet *pResourceSet = *ppResourceSet;
	if (pResourceSet->pWriteDescriptorSets == NULL) {
		pResourceSet->pWriteDescriptorSets = calloc(pInitInfo->resourceCount, sizeof(pResourceSet->pWriteDescriptorSets[0]));
	}
	for (uint32_t i = 0; i < pInitInfo->pGraphicsInstance->descriptorSetsCount; ++i) {
		for (uint32_t j = 0; j < pInitInfo->resourceCount; ++j) {
			// Not sure if it works correctly. May cause issues
			pResourceSet->ppResources[pInitInfo->ppResources[j]->binding] = pInitInfo->ppResources[j];
			//
			
			pResourceSet->pWriteDescriptorSets[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			if (pInitInfo->isMeshResourceSet) {
				pResourceSet->pWriteDescriptorSets[j].dstSet = pInitInfo->pGraphicsInstance->ppDescriptorSets[pInitInfo->meshResourceSetID][pResourceSet->setIndex];
			} else {
				pResourceSet->pWriteDescriptorSets[j].dstSet = pInitInfo->pGraphicsInstance->ppDescriptorSets[i][pResourceSet->setIndex];
			}
			pResourceSet->pWriteDescriptorSets[j].dstBinding = pInitInfo->ppResources[j]->binding;
			pResourceSet->pWriteDescriptorSets[j].descriptorCount = 1;

			VkDescriptorImageInfo *pImageInfo = VK_NULL_HANDLE;
			VkDescriptorBufferInfo *pBufferInfo = VK_NULL_HANDLE;
			if (pInitInfo->ppResources[j]->type == SG_RESOURCE_TYPE_TEXTURE_2D) {
				/* Won't work unless image resource creation is completed */
				pImageInfo = malloc(sizeof(pImageInfo[0]));
				pImageInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				pImageInfo->imageView = pInitInfo->ppResources[j]->imageView;
				pImageInfo->sampler   = pInitInfo->ppResources[j]->imageSampler;
				pResourceSet->pWriteDescriptorSets[j].pImageInfo = pImageInfo;
			} else {
				pBufferInfo = malloc(sizeof(pBufferInfo[0]));
				pBufferInfo->buffer = pInitInfo->ppResources[j]->dataBuffer.buffer;
				pBufferInfo->offset = 0;
				pBufferInfo->range = pInitInfo->ppResources[j]->dataBuffer.size;
				pResourceSet->pWriteDescriptorSets[j].pBufferInfo = pBufferInfo;
			}

			switch (pInitInfo->ppResources[j]->type) {
				case (SG_RESOURCE_TYPE_UNIFORM):
					pResourceSet->pWriteDescriptorSets[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_MESH):
					pResourceSet->pWriteDescriptorSets[j].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case (SG_RESOURCE_TYPE_TEXTURE_2D):
					pResourceSet->pWriteDescriptorSets[j].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					break;
			}

		}
		vkUpdateDescriptorSets(pApp->device, pInitInfo->resourceCount, pResourceSet->pWriteDescriptorSets, 0, VK_NULL_HANDLE);
		if (pInitInfo->isMeshResourceSet) {
			break;
		}
	}
	*ppResourceSet = pResourceSet;
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
// Comment: I guess, I meant that render pass and corresponding framebuffer content should be
// changed from one place and thus everything in this function should change from the changes
// in the input params... Or something along those lines. Should really teach myself to leave
// better comments next time. Even at the times of frustration.
SgResult createRenderPass(const SgApp *pApp, VkRenderPass* pRenderPass) {
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
		log_info("[Graphics Instance]: Render Pass Created");
	} else {
		log_error("[Graphics Instance]: Render Pass Creation Failure");
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
		/* Begin command buffer */
		VkCommandBuffer commandBuffer;
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    		.commandPool = pApp->pCommandPools[2],
    		.commandBufferCount = 1,
		};

    	
    	vkAllocateCommandBuffers(pApp->device, &allocInfo, &commandBuffer);

    	VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		};

    	vkBeginCommandBuffer(commandBuffer, &beginInfo);
		/**/


		VkImageMemoryBarrier barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pSwapchain->depthImage.image,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		};

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

		
		/* End command buffer */
		vkEndCommandBuffer(commandBuffer);

    	VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    		.commandBufferCount = 1,
    		.pCommandBuffers = &commandBuffer,
		};

    	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    	vkQueueWaitIdle(pApp->graphicsQueue);

    	vkFreeCommandBuffers(pApp->device, pApp->pCommandPools[2], 1, &commandBuffer);
		/**/
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
	pSwapchain->pFrameBuffers = malloc(sizeof(pSwapchain->pFrameBuffers[0]) * pSwapchain->imageCount);


	for (uint32_t i = 0; i < pSwapchain->imageCount; ++i) {
		VkImageView pAttachments[] = {pSwapchain->blendImageView.imageView, pSwapchain->depthImageView.imageView, pSwapchain->pFrameImageViews[i]};
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
	createRenderPass(pApp, &pGraphicsInstance->renderPass);
	
	/* Create Pipeline Layout */
	uint32_t hasMeshDescriptors = (pCreateInfo->meshSetCount > 0) ? 1: 0;
	uint32_t baseSetCount = (hasMeshDescriptors) ? pCreateInfo->meshSetCount : 1;

	pGraphicsInstance->pDescriptorSetLayouts = malloc(sizeof(pGraphicsInstance->pDescriptorSetLayouts[0]) * (pCreateInfo->setCount + hasMeshDescriptors));

	for (uint32_t i = 0; i < pCreateInfo->setCount; ++i) {
		pGraphicsInstance->pDescriptorSetLayouts[pCreateInfo->ppSets[i]->setIndex] = pCreateInfo->ppSets[i]->setLayout;
	}
	if (hasMeshDescriptors) {
		pGraphicsInstance->pDescriptorSetLayouts[pCreateInfo->ppMeshSets[0]->setIndex] = pCreateInfo->ppMeshSets[0]->setLayout;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	    .setLayoutCount = pCreateInfo->setCount + hasMeshDescriptors,
	    .pSetLayouts = pGraphicsInstance->pDescriptorSetLayouts,
	};
	if(vkCreatePipelineLayout(pApp->device, &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &pGraphicsInstance->pipelineLayout) == VK_SUCCESS) {
		log_info("[Graphics Instance]: Grapics Pipeline Layout Initialized");
	} else {
		log_error("[Graphics Instance]: Grapics Pipeline Layout Initialization Error");
	}

	pGraphicsInstance->setCount = pCreateInfo->setCount + hasMeshDescriptors;


	/* Create Descriptor Sets */
	uint32_t poolSizeCount = 0;
	for (uint32_t i = 0; i < baseSetCount; ++i) {
		for (uint32_t j = 0; j < pCreateInfo->setCount; ++j) {
			poolSizeCount += pCreateInfo->ppSets[j]->resourceCount;
		}
		poolSizeCount += (hasMeshDescriptors)
			? pCreateInfo->ppMeshSets[i]->resourceCount 
			: 0;
	}
	/* Num of pool sizes = count of the resources in each non-mesh
	 * specific descriptor set * the amount of mesh descriptor sets if there
	 * are some + count of the resources in mesh-specific descriptor sets
	 * */
	if (poolSizeCount == 0) {
		log_warn("[Graphics Instance]: No descriptor pools");
	}
	uint32_t offset = 0;
	VkDescriptorPoolSize *pPoolSizes = calloc(poolSizeCount, sizeof(pPoolSizes[0]));
	for (uint32_t i = 0; i < baseSetCount; ++i) {
		for (uint32_t j = 0; j < pCreateInfo->setCount; ++j) {
			for (uint32_t k = 0; k < pCreateInfo->ppSets[j]->resourceCount; ++k) {
				switch (pCreateInfo->ppSets[j]->ppResources[k]->type) {
				case SG_RESOURCE_TYPE_TEXTURE_2D:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
				case SG_RESOURCE_TYPE_MESH:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case SG_RESOURCE_TYPE_UNIFORM:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;
				}
				pPoolSizes[offset].descriptorCount = 1;
				++offset;
			}
		}

		if (hasMeshDescriptors) {
		for (uint32_t j = 0; j < pCreateInfo->ppMeshSets[i]->resourceCount; ++j) {
			switch (pCreateInfo->ppMeshSets[i]->ppResources[j]->type) {
				case SG_RESOURCE_TYPE_TEXTURE_2D:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					break;
				case SG_RESOURCE_TYPE_MESH:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					break;
				case SG_RESOURCE_TYPE_UNIFORM:
					pPoolSizes[offset].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					break;

			}
			pPoolSizes[offset].descriptorCount = 1;
			++offset;
		}
		}
	}
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
	    .pPoolSizes = pPoolSizes,
	    .poolSizeCount = poolSizeCount,
	    .maxSets = (pCreateInfo->setCount + hasMeshDescriptors)* ((hasMeshDescriptors) ? pCreateInfo->meshSetCount : 1 ),
	};

	vkCreateDescriptorPool(pApp->device, &descriptorPoolCreateInfo, VK_NULL_HANDLE, &pGraphicsInstance->descriptorPool);
	free(pPoolSizes);
	pGraphicsInstance->descriptorSetsCount = baseSetCount;
	pGraphicsInstance->ppDescriptorSets = malloc(pGraphicsInstance->descriptorSetsCount * sizeof(pGraphicsInstance->ppDescriptorSets[0]));
	for (uint32_t i = 0; i < pGraphicsInstance->descriptorSetsCount; ++i) {
		pGraphicsInstance->ppDescriptorSets[i] = malloc((pCreateInfo->setCount + hasMeshDescriptors) * sizeof(pGraphicsInstance->ppDescriptorSets[0][0]));
	}
	VkDescriptorSetLayout *pDescriptorSetLayouts = malloc(sizeof(pDescriptorSetLayouts[0]) * (pGraphicsInstance->setCount));
	for (uint32_t i = 0; i < pCreateInfo->setCount; ++i) {
		pDescriptorSetLayouts[pGraphicsInstance->ppSets[i]->setIndex] = pGraphicsInstance->ppSets[i]->setLayout;
	}
	if (hasMeshDescriptors) {
		pDescriptorSetLayouts[pCreateInfo->ppMeshSets[0]->setIndex] = pCreateInfo->ppMeshSets[0]->setLayout;
	}
	VkDescriptorSetAllocateInfo descrriptorSetAllocInfo = {
	    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
	    .descriptorPool = pGraphicsInstance->descriptorPool,
	    .descriptorSetCount = pGraphicsInstance->setCount,
	    .pSetLayouts = pDescriptorSetLayouts,
	};
	for (uint32_t i = 0; i < pGraphicsInstance->descriptorSetsCount; ++i) {
		vkAllocateDescriptorSets(pApp->device, &descrriptorSetAllocInfo, pGraphicsInstance->ppDescriptorSets[i]);
	}
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

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineVertexInputStateCreateInfo vertexInput = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineViewportStateCreateInfo viewportstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
	    .viewportCount = 1,
	    .scissorCount = 1,
	};

	VkPipelineMultisampleStateCreateInfo multisamplestate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	    .rasterizationSamples = pApp->msaaSampleCount,
		.minSampleShading = 1.0f,
	};

	VkPipelineDepthStencilStateCreateInfo depthstencilstate = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
	    .depthTestEnable = VK_TRUE,
	    .depthWriteEnable = VK_TRUE,
	    .depthCompareOp = VK_COMPARE_OP_LESS,
	};

	VkPipelineColorBlendAttachmentState colorAttachmentState = {
		 .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,

		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendState = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_COPY,
	    .attachmentCount = 1,
	    .pAttachments = &colorAttachmentState,
		.blendConstants[0] = 0.0f,
		.blendConstants[1] = 0.0f,
		.blendConstants[2] = 0.0f,
		.blendConstants[3] = 0.0f,
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

	    .pInputAssemblyState = &inputAssembly,
	    .pVertexInputState = &vertexInput,

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

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands** ppUpdateCommands) {

	vkDeviceWaitIdle(pInitInfo->pApp->device);
	SgUpdateCommands *pUpdateCommands = calloc(1, sizeof(pUpdateCommands[0]));
	pUpdateCommands->pCommandBuffers = calloc(pInitInfo->pGraphicsInstance->swapchain.imageCount, sizeof(pUpdateCommands[0]));

	VkCommandBufferAllocateInfo commandAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    	.commandPool = pInitInfo->pApp->pCommandPools[0],
    	.commandBufferCount = pInitInfo->pGraphicsInstance->swapchain.imageCount,
		.pNext = NULL,
	};
	vkAllocateCommandBuffers(pInitInfo->pApp->device, &commandAllocInfo, pUpdateCommands->pCommandBuffers);

	VkClearValue pClearValues[] = {
		{.color = {{0.04, 0.01, 0, 1}}},
		{.depthStencil = {1.0f, 0}},
	};
	for (uint32_t i = 0; i < pInitInfo->pGraphicsInstance->swapchain.imageCount; ++i) {
		VkRenderPassBeginInfo renderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.framebuffer = pInitInfo->pGraphicsInstance->swapchain.pFrameBuffers[i],
			.renderPass = pInitInfo->pGraphicsInstance->renderPass,
			.renderArea.extent = pInitInfo->pGraphicsInstance->swapchain.extent,
			.clearValueCount = NUMOF(pClearValues),
			.pClearValues = pClearValues,
		};

		// TODO: RESIZE SHOULD BE DYNAMIC! YOU YOURSELF MADE IT THAT WAY!
		// YET NOW YOU ARE TOO LAZY TO MAKE IT A THING. WORK!
		VkRect2D scissor = (VkRect2D){
		    { 0, 0, },
			pInitInfo->pGraphicsInstance->swapchain.extent,
		};
		VkViewport viewport = (VkViewport)
		{ 0, 0, (float)pInitInfo->pGraphicsInstance->swapchain.extent.width, (float)pInitInfo->pGraphicsInstance->swapchain.extent.height, 0, 1};

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};
		vkBeginCommandBuffer(pUpdateCommands->pCommandBuffers[i], &commandBufferBeginInfo); 

		/* */
		vkCmdSetViewport(pUpdateCommands->pCommandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(pUpdateCommands->pCommandBuffers[i], 0, 1, &scissor);
		vkCmdBindPipeline(pUpdateCommands->pCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pInitInfo->pGraphicsInstance->graphicsPipeline);

		vkCmdBeginRenderPass(pUpdateCommands->pCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		for (uint32_t j = 0; j < pInitInfo->indexResourceCount; ++j) {

			vkCmdBindDescriptorSets(pUpdateCommands->pCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pInitInfo->pGraphicsInstance->pipelineLayout, 0, pInitInfo->pGraphicsInstance->setCount, pInitInfo->pGraphicsInstance->ppDescriptorSets[j], 0, NULL);
			vkCmdBindIndexBuffer(pUpdateCommands->pCommandBuffers[i], pInitInfo->ppIndexResources[j]->dataBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(pUpdateCommands->pCommandBuffers[i], pInitInfo->ppIndexResources[j]->dataBuffer.size/sizeof(uint32_t), 1, 0, 0, 0);

		}
		vkCmdEndRenderPass(pUpdateCommands->pCommandBuffers[i]);
		vkEndCommandBuffer(pUpdateCommands->pCommandBuffers[i]);
	}
	*ppUpdateCommands = pUpdateCommands;

	return SG_SUCCESS;
}

SgBool sgAppUpdate(const SgAppUpdateInfo* pUpdateInfo) {
	SgApp *pApp = pUpdateInfo->pApp;
	SgGraphicsInstance *pGraphicsInstance = pUpdateInfo->pGraphicsInstance;
	if(glfwWindowShouldClose(pApp->pWindow))
		return 0;
	glfwPollEvents();
	/* Retrieve Image */
    vkWaitForFences(pApp->device, 1, &pApp->pFrameFences[pApp->currentFrame], VK_TRUE, UINT64_MAX);

	VkResult res = vkAcquireNextImageKHR(pApp->device, pGraphicsInstance->swapchain.swapchain, UINT64_MAX, pApp->pFrameReadySemaphore[pApp->currentFrame], VK_NULL_HANDLE, &pApp->frameImageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		log_warn("[TODO]: RESIZE");
	} else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		log_error("[App Update]: Frame Image acquisition failure");
	}

	/* Draw to Frame Image */
	if (pApp->pFrameImageInFlightFences[pApp->frameImageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(pApp->device, 1, &pApp->pFrameImageInFlightFences[pApp->currentFrame], VK_TRUE, UINT64_MAX);
	}
	pApp->pFrameImageInFlightFences[pApp->frameImageIndex] = pApp->pFrameFences[pApp->currentFrame];

    VkSemaphore pWaitSemaphores[] = {pApp->pFrameReadySemaphore[pApp->currentFrame]};
    VkPipelineStageFlags pWaitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

	VkSubmitInfo submitInfo = {
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .waitSemaphoreCount = NUMOF(pWaitSemaphores),
	    .pWaitSemaphores = pWaitSemaphores,
	    .pWaitDstStageMask = pWaitStages,
	    .commandBufferCount = 1,
	    .pCommandBuffers = &pUpdateInfo->pUpdateCommands->pCommandBuffers[pApp->frameImageIndex],
	    .signalSemaphoreCount = 1,
	    .pSignalSemaphores = &pApp->pFrameFinishedSemaphore[pApp->currentFrame],
	};

    vkResetFences(pApp->device, 1, &pApp->pFrameFences[pApp->currentFrame]);

	if (vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pApp->pFrameFences[pApp->currentFrame])) {
		log_warn("[Queue Submit]: Draw command error");
	}

	VkPresentInfoKHR presentInfo = {
	    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .swapchainCount = 1,
	    .pSwapchains = &pGraphicsInstance->swapchain.swapchain,
	    .pImageIndices = &pApp->frameImageIndex,
	    .waitSemaphoreCount = 1,
	    .pWaitSemaphores = &pApp->pFrameFinishedSemaphore[pApp->currentFrame],
	};
	vkQueuePresentKHR(pApp->graphicsQueue, &presentInfo);

	pApp->currentFrame = (pApp->currentFrame + 1) % SG_FRAME_QUEUE_LENGTH;
	return 1;
}

void sgDestroyShader(const SgApp *pApp, SgShader **ppShader) {
	SgShader *pShader = *ppShader;
	vkDeviceWaitIdle(pApp->device);
	vkDestroyShaderModule(pApp->device, pShader->shader, VK_NULL_HANDLE);
	free(pShader);
	ppShader = NULL;
}

void sgDestroyResource(const SgApp *pApp, SgResource **ppResource) {
	vkDeviceWaitIdle(pApp->device);
	SgResource* pResource = *ppResource;
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		vkDestroyImageView(pApp->device, pResource->imageView, VK_NULL_HANDLE);
		vmaUnmapMemory(pApp->allocator, pResource->image.allocation);
	} else {
		if (pResource->type & SG_RESOURCE_TYPE_REQIRE_STAGING_MASK) {
			vmaUnmapMemory(pApp->allocator, pResource->stagingBuffer.allocation);
			vmaDestroyBuffer(pApp->allocator, pResource->stagingBuffer.buffer, pResource->stagingBuffer.allocation);
			vmaDestroyBuffer(pApp->allocator, pResource->dataBuffer.buffer, pResource->dataBuffer.allocation);
		} else {
			vmaUnmapMemory(pApp->allocator, pResource->dataBuffer.allocation);
			vmaDestroyBuffer(pApp->allocator, pResource->dataBuffer.buffer, pResource->dataBuffer.allocation);
		}
	}
	vkDestroyCommandPool(pApp->device, pResource->commandPool, VK_NULL_HANDLE);
	free(pResource);
	ppResource = NULL;
}

void sgDestroyResourceSet(const SgApp *pApp, SgResourceSet** ppResourceSet) {
	SgResourceSet *pResourceSet = *ppResourceSet;
	vkDestroyDescriptorSetLayout(pApp->device, pResourceSet->setLayout, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < pResourceSet->resourceCount; ++i) {
		if (pResourceSet->pWriteDescriptorSets[i].pImageInfo) {
			free((VkDescriptorImageInfo*) pResourceSet->pWriteDescriptorSets[i].pImageInfo);
		} 
		if (pResourceSet->pWriteDescriptorSets[i].pBufferInfo) {
			free((VkDescriptorBufferInfo*) pResourceSet->pWriteDescriptorSets[i].pBufferInfo);
		} 
		if (pResourceSet->pWriteDescriptorSets[i].pTexelBufferView) {
			free((void*) pResourceSet->pWriteDescriptorSets[i].pTexelBufferView);
		}
	}
	free(pResourceSet->pWriteDescriptorSets);
	free(pResourceSet->pSetLayoutBindings);
	free(pResourceSet);
	ppResourceSet = NULL;
}

void sgDeinitUpdateCommands(const SgApp *pApp, SgUpdateCommands** ppUpdateCommands) {
	SgUpdateCommands *pUpdateCommands = *ppUpdateCommands;
	free(pUpdateCommands->pCommandBuffers);
	free(pUpdateCommands);
	ppUpdateCommands = NULL;
}

void sgDestroyGraphicsInstance(const SgApp *pApp, SgGraphicsInstance **ppGraphicsInstance) {
	SgGraphicsInstance *pGraphicsInstance = *ppGraphicsInstance;
	vkDeviceWaitIdle(pApp->device);

	vkResetDescriptorPool(pApp->device, pGraphicsInstance->descriptorPool, 0);
	vkDestroyDescriptorPool(pApp->device, pGraphicsInstance->descriptorPool, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < pGraphicsInstance->descriptorSetsCount; ++i) {
		free(pGraphicsInstance->ppDescriptorSets[i]);
	}
	free(pGraphicsInstance->ppDescriptorSets);

	vkDestroyPipelineLayout(pApp->device, pGraphicsInstance->pipelineLayout, VK_NULL_HANDLE);
	vkDestroyPipeline(pApp->device, pGraphicsInstance->graphicsPipeline, VK_NULL_HANDLE);
	vkDestroyRenderPass(pApp->device, pGraphicsInstance->renderPass, VK_NULL_HANDLE);

	vmaDestroyImage(pApp->allocator, pGraphicsInstance->swapchain.depthImage.image, pGraphicsInstance->swapchain.depthImage.allocation);
	vkDestroyImageView(pApp->device, pGraphicsInstance->swapchain.blendImageView.imageView, VK_NULL_HANDLE);
	vmaDestroyImage(pApp->allocator, pGraphicsInstance->swapchain.blendImage.image, pGraphicsInstance->swapchain.blendImage.allocation);
	vkDestroyImageView(pApp->device, pGraphicsInstance->swapchain.depthImageView.imageView, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < pGraphicsInstance->swapchain.imageCount; ++i) {
		vkDestroyImageView(pApp->device, pGraphicsInstance->swapchain.pFrameImageViews[i], VK_NULL_HANDLE);
		vkDestroyFramebuffer(pApp->device, pGraphicsInstance->swapchain.pFrameBuffers[i], VK_NULL_HANDLE);
	}
	free(pGraphicsInstance->swapchain.pFrameImages);
	free(pGraphicsInstance->swapchain.pFrameImageViews);
	free(pGraphicsInstance->swapchain.pFrameBuffers);

	vkDestroySwapchainKHR(pApp->device, pGraphicsInstance->swapchain.swapchain, VK_NULL_HANDLE);

	free(pGraphicsInstance->pDescriptorSetLayouts);

	free(pGraphicsInstance);
	ppGraphicsInstance = NULL;
}

void sgDestroyApp(SgApp **ppApp) {
	SgApp* pApp = *ppApp;
	vkDeviceWaitIdle(pApp->device);
	
	for (uint32_t i = 0; i < SG_THREADS_COUNT * SG_FRAME_QUEUE_LENGTH; ++i) {
		vkResetCommandPool(pApp->device, pApp->pCommandPools[i], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
	for (uint32_t i = 0; i < SG_FRAME_QUEUE_LENGTH; ++i) {
		vkDestroySemaphore(pApp->device, pApp->pFrameReadySemaphore[i], VK_NULL_HANDLE);
		vkDestroySemaphore(pApp->device, pApp->pFrameFinishedSemaphore[i], VK_NULL_HANDLE);
		vkDestroyFence(pApp->device, pApp->pFrameFences[i], VK_NULL_HANDLE);
	}

	for (uint32_t i = 0; i < SG_FRAME_QUEUE_LENGTH * SG_THREADS_COUNT; ++i) {
		vkDestroyCommandPool(pApp->device, pApp->pCommandPools[i], VK_NULL_HANDLE);
	}

	vmaDestroyAllocator(pApp->allocator);

	vkDestroyDevice(pApp->device, VK_NULL_HANDLE);
	vkDestroySurfaceKHR(pApp->instance, pApp->surface, VK_NULL_HANDLE);
	vkDestroyInstance(pApp->instance, VK_NULL_HANDLE);
	glfwDestroyWindow(pApp->pWindow);
	glfwTerminate();
	free(pApp);
	ppApp = NULL;
}
