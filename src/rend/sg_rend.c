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
		sgLogInfo_Debug("[AppInit]: GLFW window created");
	} else {
		sgLogFatal("[AppInit]: GLFW window creation error");
	}

	if (pCreateInfo->flags & SG_APP_CURSOR_HIDDEN) {
		glfwSetInputMode(pApp->pWindow , GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	glfwSetInputMode(pApp->pWindow, GLFW_STICKY_KEYS, GLFW_TRUE);
	return SG_SUCCESS;
}

SgResult createVkInstance(const SgAppCreateInfo *pCreateInfo, SgApp *pApp) {
	/* Instance */
    uint32_t apiver = volkGetInstanceVersion();
    if (apiver >= VK_API_VERSION_1_2) {
		sgLogInfo_Debug("[AppInit]: Vulkan API version 1.2 of newer found");
	} else {
		sgLogError("[AppInit]: Vulkan API version is too old");
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
		sgLogInfo_Debug("[AppInit]: Vulkan Instance created");
	} else { 
		sgLogFatal("[AppInit]: Vulkan Instance creation failure");
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
	SgApp* pApp;
	SG_CALLOC_NUM(pApp, 1);

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
		sgLogError("[App Init]: Synchonization Primitive init failure");
	}

	/* End */
	*ppSgApp = pApp;

	return SG_SUCCESS;
}

SgWindow* sgGetWindow(SgApp *pApp) {
	return pApp->pWindow;
}


SgResult sgCreateShader(const SgApp *pApp, const SgShaderCreateInfo* pCreateInfo, SgShader **ppShader) {
	SgShader* pShader;
	SG_CALLOC_NUM(pShader, 1);
	SgFile* pFile = pCreateInfo->pFile;
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = pFile->size,
		.pCode = pFile->pBytes,
	};
	VkResult res = vkCreateShaderModule(pApp->device, &createInfo, VK_NULL_HANDLE, &pShader->shader);

	if(res == VK_SUCCESS) {
		sgLogInfo_Debug("[Shader]: Shader Module created successfully");
	} else {
		sgLogError("[Shader]: Shader Module creation failure");
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
		sgLogInfo_Debug("[Graphics Instance]: Render Pass Created");
	} else {
		sgLogError("[Graphics Instance]: Render Pass Creation Failure");
	}

	return SG_SUCCESS;
}

struct bindRenderObjectInfo {
	SgMaterialMap*   pMaterialMap;
	VkCommandBuffer* pCommandBuffer;
	SgMeshSet*       pMeshSet;
};

_Bool renderPassBindRenderObjects(const void* item, void* udata) {
	const SgMaterialRenderObjects* pRenderObjects = item;
	struct bindRenderObjectInfo* pInfo = udata;

	SgMaterial materialGet = {
		.pName = pRenderObjects->materialName,
	};

	SgMaterial* pMaterial = hashmap_get(pInfo->pMaterialMap->pMaterialMap, &materialGet);

	vkCmdBindPipeline(*pInfo->pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pMaterial->pipeline);
	vkCmdBindDescriptorSets(*pInfo->pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pMaterial->pipelineLayout, 0, pMaterial->descriptorSetCount, pMaterial->pDescriptorSets, 0, NULL);
	for (uint32_t i = 0; i < pRenderObjects->renderObjectCount; ++i) {
		vkCmdDrawIndexed(*pInfo->pCommandBuffer, pInfo->pMeshSet->pIndexSizes[pRenderObjects->pRenderObjects[i].meshID], 1, 0, pInfo->pMeshSet->pVertexOffsets[pRenderObjects->pRenderObjects[i].meshID], 0);
	}
	

	return 1;
}

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands** ppUpdateCommands) {

	vkDeviceWaitIdle(pInitInfo->pMaterialMap->pApp->device);
	SgUpdateCommands *pUpdateCommands;
	SG_CALLOC_NUM(pUpdateCommands, 1);
	SG_CALLOC_NUM(pUpdateCommands->pCommandBuffers, pInitInfo->pMaterialMap->swapchain.imageCount);

	VkCommandBufferAllocateInfo commandAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    	.commandPool = pInitInfo->pMaterialMap->pApp->pCommandPools[0],
    	.commandBufferCount = pInitInfo->pMaterialMap->swapchain.imageCount,
		.pNext = NULL,
	};
	vkAllocateCommandBuffers(pInitInfo->pMaterialMap->pApp->device, &commandAllocInfo, pUpdateCommands->pCommandBuffers);

	VkClearValue pClearValues[] = {
		{.color = {{0.04, 0.01, 0, 1}}},
		{.depthStencil = {1.0f, 0}},
	};
	for (uint32_t i = 0; i < pInitInfo->pMaterialMap->swapchain.imageCount; ++i) {
		VkRenderPassBeginInfo renderPassBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.framebuffer = pInitInfo->pMaterialMap->swapchain.pFrameBuffers[i],
			.renderPass = pInitInfo->pMaterialMap->renderPass,
			.renderArea.extent = pInitInfo->pMaterialMap->swapchain.extent,
			.clearValueCount = NUMOF(pClearValues),
			.pClearValues = pClearValues,
		};

		// TODO: RESIZE SHOULD BE DYNAMIC! YOU YOURSELF MADE IT THAT WAY!
		// YET NOW YOU ARE TOO LAZY TO MAKE IT A THING. WORK!
		pInitInfo->pMaterialMap->pApp->scissor = (VkRect2D){
		    { 0, 0, },
			pInitInfo->pMaterialMap->swapchain.extent,
		};
		pInitInfo->pMaterialMap->pApp->viewport = (VkViewport)
		{ 0, 0, (float)pInitInfo->pMaterialMap->swapchain.extent.width, (float)pInitInfo->pMaterialMap->swapchain.extent.height, 0, 1};

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};
		vkBeginCommandBuffer(pUpdateCommands->pCommandBuffers[i], &commandBufferBeginInfo); 

		/* */
		vkCmdSetViewport(pUpdateCommands->pCommandBuffers[i], 0, 1, &pInitInfo->pMaterialMap->pApp->viewport);
		vkCmdSetScissor(pUpdateCommands->pCommandBuffers[i], 0, 1, &pInitInfo->pMaterialMap->pApp->scissor);
		vkCmdBeginRenderPass(pUpdateCommands->pCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindIndexBuffer(pUpdateCommands->pCommandBuffers[i], pInitInfo->pMeshSet->indicesResource->dataBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

		struct bindRenderObjectInfo renderObjectInfo = {
			.pMeshSet = pInitInfo->pMeshSet,
			.pMaterialMap = pInitInfo->pMaterialMap,
			.pCommandBuffer = &pUpdateCommands->pCommandBuffers[i],
		};
		hashmap_scan(pInitInfo->pMaterialMap->pMaterialRenderObjectMap, renderPassBindRenderObjects, &renderObjectInfo);
		vkCmdEndRenderPass(pUpdateCommands->pCommandBuffers[i]);
		vkEndCommandBuffer(pUpdateCommands->pCommandBuffers[i]);
	}
	*ppUpdateCommands = pUpdateCommands;

	return SG_SUCCESS;
}

SgBool sgAppUpdate(SgAppUpdateInfo* pUpdateInfo) {
	SgApp *pApp = pUpdateInfo->pMaterialMap->pApp;
	SgSwapchain* pSwapchain = &pUpdateInfo->pMaterialMap->swapchain;
	if(glfwWindowShouldClose(pApp->pWindow))
		return 0;
	glfwPollEvents();
	/* Retrieve Image */
    vkWaitForFences(pApp->device, 1, &pApp->pFrameFences[pApp->currentFrame], VK_TRUE, UINT64_MAX);
	{
		int width, height;
		glfwGetWindowSize(pApp->pWindow, &width, &height);
		pSwapchain->extent.height = height;
		pSwapchain->extent.width = width;

		pApp->scissor = (VkRect2D){
		    { 0, 0, },
			pUpdateInfo->pMaterialMap->swapchain.extent,
		};
		pApp->viewport = (VkViewport)
		{ 
			0, 0,
			(float)pSwapchain->extent.width, (float)pSwapchain->extent.height, 0, 1
		};
	}


	VkResult res = vkAcquireNextImageKHR(pApp->device, pSwapchain->swapchain, UINT64_MAX, pApp->pFrameReadySemaphore[pApp->currentFrame], VK_NULL_HANDLE, &pApp->frameImageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR) {
		sgLogWarn_Debug("[TODO]: RESIZE");
	} else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		sgLogError("[App Update]: Frame Image acquisition failure");
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
		sgLogError("[Queue Submit]: Draw command error");
	}

	VkPresentInfoKHR presentInfo = {
	    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
	    .swapchainCount = 1,
	    .pSwapchains = &pSwapchain->swapchain,
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
	if (pResource->resourceBinding.type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		vkDestroyImageView(pApp->device, pResource->imageView, VK_NULL_HANDLE);
		vkDestroySampler(pApp->device, pResource->imageSampler, VK_NULL_HANDLE);
		if (pResource->stagingBuffer.allocation) {
			vmaUnmapMemory(pApp->allocator, pResource->stagingBuffer.allocation);
			vmaDestroyBuffer(pApp->allocator, pResource->stagingBuffer.buffer, pResource->stagingBuffer.allocation);
		} else {
			vmaUnmapMemory(pApp->allocator, pResource->image.allocation);
		}
		vmaDestroyImage(pApp->allocator, pResource->image.image, pResource->image.allocation);
	} else {
		if (pResource->resourceBinding.type & SG_RESOURCE_TYPE_REQIRE_STAGING_MASK) {
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

void sgDeinitUpdateCommands(const SgApp *pApp, SgUpdateCommands** ppUpdateCommands) {
	SgUpdateCommands *pUpdateCommands = *ppUpdateCommands;
	free(pUpdateCommands->pCommandBuffers);
	free(pUpdateCommands);
	ppUpdateCommands = NULL;
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
#ifdef _DEBUG
	vkDestroyDebugUtilsMessengerEXT(pApp->instance, pApp->debugCallback, VK_NULL_HANDLE);
#endif
	vkDestroyInstance(pApp->instance, VK_NULL_HANDLE);
	glfwDestroyWindow(pApp->pWindow);
	glfwTerminate();
	free(pApp);
	ppApp = NULL;
}
