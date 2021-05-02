#include "sg_rend.h"
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

static SgResult getAppConfig(const SgFile* pFile, SgAppConfig* pAppConfig) {
	const char *error_ptr;
	cJSON* configJSON = pAppConfig->configJSON;

	if (configJSON == NULL) {
		configJSON = cJSON_ParseWithOpts((char*) pFile->pBytes, &error_ptr, 1);
		if (configJSON == NULL && error_ptr != NULL) {
    	    log_error("[JSON]: Error before: %s\n", error_ptr);
			return -1;
		}
    }
	cJSON* window = cJSON_GetObjectItem(configJSON, "window");
	{
		cJSON* width = cJSON_GetObjectItem(window, "width");
		pAppConfig->windowConfig.dimentions[0] = cJSON_GetNumberValue(width);
		cJSON* height = cJSON_GetObjectItem(window, "height");
		pAppConfig->windowConfig.dimentions[1] = cJSON_GetNumberValue(height);
		cJSON* cursor = cJSON_GetObjectItem(window, "cursor");
		if (!cJSON_GetNumberValue(cursor)) {
			pAppConfig->windowConfig.createFlags |= SG_APP_CURSOR_HIDDEN;
		}
		cJSON* fullscreen = cJSON_GetObjectItem(window, "fullscreen");
		if (cJSON_GetNumberValue(fullscreen)) {
			pAppConfig->windowConfig.createFlags |= SG_APP_WINDOW_FULLSCREEN;
		}
		cJSON* resizable = cJSON_GetObjectItem(window, "resizable");
		if (cJSON_GetNumberValue(resizable)) {
			pAppConfig->windowConfig.createFlags |= SG_APP_WINDOW_RESIZABLE;
		}

	}
	cJSON* graphics = cJSON_GetObjectItem(configJSON, "graphics");
	{
		cJSON* msaa = cJSON_GetObjectItem(graphics, "msaa");
		pAppConfig->graphicsConfig.mssa = cJSON_GetNumberValue(msaa);
		cJSON* vsync = cJSON_GetObjectItem(graphics, "vsync");
	}
	pAppConfig->configJSON = configJSON;

	return SG_SUCCESS;
}

static SgResult createGLFWwindow(const SgAppConfig *pAppConfig, SgWindow **ppWindow) {
	SgWindow* pWindow = *ppWindow;
	glfwWindowHint(GLFW_RESIZABLE, pAppConfig->windowConfig.createFlags & SG_APP_WINDOW_RESIZABLE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	if (pAppConfig->windowConfig.createFlags & SG_APP_WINDOW_FULLSCREEN) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		 
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

		pWindow = glfwCreateWindow(mode->width, mode->height, pAppConfig->windowConfig.pName, monitor, NULL);
	} else {
		pWindow = glfwCreateWindow(pAppConfig->windowConfig.dimentions[0], pAppConfig->windowConfig.dimentions[1], pAppConfig->windowConfig.pName, NULL, NULL);
	}
	if(pWindow) {
		sgLogInfo_Debug("[AppInit]: GLFW window created");
	} else {
		sgLogFatal("[AppInit]: GLFW window creation error");
	}

	if (pAppConfig->windowConfig.createFlags & SG_APP_CURSOR_HIDDEN) {
		glfwSetInputMode(pWindow , GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	glfwSetInputMode(pWindow, GLFW_STICKY_KEYS, GLFW_TRUE);
	*ppWindow = pWindow;
	return SG_SUCCESS;
}

static SgResult createVkInstance(const SgAppCreateInfo *pCreateInfo, VkInstance *pInstance) {
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
    VkResult vkRes = vkCreateInstance(&createInfo, VK_NULL_HANDLE, pInstance);
	if (vkRes == VK_SUCCESS) {
		sgLogInfo_Debug("[AppInit]: Vulkan Instance created");
	} else { 
		sgLogFatal("[AppInit]: Vulkan Instance creation failure");
	}

	volkLoadInstance(*pInstance);

    free(pExt);
	return SG_SUCCESS;
}
static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT)  { return VK_SAMPLE_COUNT_8_BIT;  }
    if (counts & VK_SAMPLE_COUNT_4_BIT)  { return VK_SAMPLE_COUNT_4_BIT;  }
    if (counts & VK_SAMPLE_COUNT_2_BIT)  { return VK_SAMPLE_COUNT_2_BIT;  }

    return VK_SAMPLE_COUNT_1_BIT;
}

void getSupportedDepthStencilFormats(const VkPhysicalDevice* pPhysicalDevice, VkFormat* pLowDepthStencil, VkFormat* pHighDepthStencil) {
	VkFormatProperties props;
	VkFormat formats[] = {VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
	for (uint32_t i = 0; i < NUMOF(formats); ++i) {
		vkGetPhysicalDeviceFormatProperties(*pPhysicalDevice, formats[i], &props);
		if (props.optimalTilingFeatures && *pLowDepthStencil == 0) {
			*pLowDepthStencil = formats[i];
			*pHighDepthStencil = formats[i];
		}
	}
	return;
}

SgResult sgCreateApp(const SgAppCreateInfo *pCreateInfo, SgApp **ppApp) {
	/* TODO: Implement custom allocation callbacks */
	SgApp* pApp;
	SG_CALLOC_NUM(pApp, 1);

	glfwInit();
	volkInitialize();

	pApp->appConfig.windowConfig.pName = pCreateInfo->pName;
	getAppConfig(pCreateInfo->pConfigFile, &pApp->appConfig);
	/* Window */
	createGLFWwindow(&pApp->appConfig, &pApp->pWindow);
	/* Vulkan Instance */
	createVkInstance(pCreateInfo, &pApp->instance);
#ifdef _DEBUG
	/* Vulkan Debug Callback */
	registerDebugCallback(&pApp->instance, &pApp->debugCallback);
#endif
	/* Vulkan Surface */
	createWindowSurface(&pApp->instance, pApp->pWindow, &pApp->surface);
	/* Vulkan Physical Device */
	SgPhysicalDeviceGetInfo physicalDeviceGetInfo = {
		.pInstance = &pApp->instance,
		.pSurface = &pApp->surface,
	};
	getPhysicalDevice(&physicalDeviceGetInfo, &pApp->physicalDevice);
	pApp->graphicsQueueFamilyIdx = getGraphicsFamilyIndex(pApp->physicalDevice);
	VkSampleCountFlags maxSamples = getMaxUsableSampleCount(pApp->physicalDevice);
	pApp->msaaSampleCount = (pApp->appConfig.graphicsConfig.mssa > maxSamples) ? maxSamples : pApp->appConfig.graphicsConfig.mssa;
	/* Vulkan Logical Device */
	SgLogicalDeviceGetInfo logicalDeviceGetInfo = {
		.createInfosCount = 1,
		.pQueueCreateInfos = (SgDeviceQueueCreateInfo[]){
			{
				.queueIndex = pApp->graphicsQueueFamilyIdx,
				.pQueuePriorities = (float[]) {
					1
				},
				.queueCount = 1,
			},
		},
		.physicalDevice = pApp->physicalDevice,
	};
	getSupportedDepthStencilFormats(&pApp->physicalDevice, &pApp->lowDepthStencil, &pApp->highDepthStencil);

	getLogicalDevice(&logicalDeviceGetInfo, &pApp->device);
	vkGetDeviceQueue(pApp->device, pApp->graphicsQueueFamilyIdx, 0, &pApp->graphicsQueue);

	/* Create Vulkan Allocator */
	createAllocator(&pApp->physicalDevice, &pApp->device, &pApp->instance, &pApp->allocator);

	/* Vulkan Surface Attirbutes */
	getSurfaceAttributes(&pApp->physicalDevice, &pApp->surface, &pApp->surfaceAttributes);

	/* Command Pool Initialization */
	pApp->commandPoolCount = SG_THREADS_COUNT;
	createCommandPools(&pApp->device, pApp->graphicsQueueFamilyIdx, pApp->threads, pApp->pCommandPools, pApp->commandPoolCount);

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

	SG_CALLOC_NUM(pApp->pScissor, 1);
	SG_CALLOC_NUM(pApp->pViewport, 1);

	/* End */
	*ppApp = pApp;

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
	vkCmdBindDescriptorSets(*pInfo->pCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pMaterial->pipelineLayout, 0, pMaterial->descriptorSetCount, pRenderObjects->pDescriptorSets, 0, NULL);
	for (uint32_t i = 0; i < pRenderObjects->renderObjectCount; ++i) {
		uint32_t meshID = pRenderObjects->pRenderObjects[i].meshID;
		//
		vkCmdDrawIndexed(*pInfo->pCommandBuffer, pInfo->pMeshSet->pIndexSizes[meshID], pRenderObjects->pRenderObjects[i].instanceCount, 0, pInfo->pMeshSet->pVertexOffsets[meshID], 0);
	}

	return 1;
}

SgResult sgInitUpdateCommands(const SgUpdateCommandsInitInfo *pInitInfo, SgUpdateCommands** ppUpdateCommands) {

	vkDeviceWaitIdle(pInitInfo->pMaterialMap->pApp->device);
	SgUpdateCommands *pUpdateCommands = *ppUpdateCommands;
	// TODO: fix memory leak
	SG_CALLOC_NUM(pUpdateCommands, 1);
	pUpdateCommands->pResourceMap = pInitInfo->pResourceMap;
	SG_STRETCHALLOC(pUpdateCommands->pCommandBuffers, pInitInfo->pMaterialMap->swapchain.imageCount, "Update Command Realloc Error");

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
		*pInitInfo->pMaterialMap->pApp->pScissor = (VkRect2D){
		    { 0, 0, },
			pInitInfo->pMaterialMap->swapchain.extent,
		};
		*pInitInfo->pMaterialMap->pApp->pViewport = (VkViewport)
		{ 0, 0, (float)pInitInfo->pMaterialMap->swapchain.extent.width, (float)pInitInfo->pMaterialMap->swapchain.extent.height, 0, 1};

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		};
		vkBeginCommandBuffer(pUpdateCommands->pCommandBuffers[i], &commandBufferBeginInfo); 

		/* */
		vkCmdSetViewport(pUpdateCommands->pCommandBuffers[i], 0, 1, pInitInfo->pMaterialMap->pApp->pViewport);
		vkCmdSetScissor(pUpdateCommands->pCommandBuffers[i], 0, 1, pInitInfo->pMaterialMap->pApp->pScissor);
		vkCmdBeginRenderPass(pUpdateCommands->pCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		SgResource findResource = {
			.pName = pInitInfo->pMeshSet->indicesResourceName,
		};
		SgResource* pResource = hashmap_get(pInitInfo->pResourceMap->pResourceMap, &findResource);
		if (pResource == NULL) {
			sgLogError("Index Resource Not Present");
		}
		vkCmdBindIndexBuffer(pUpdateCommands->pCommandBuffers[i], pResource->dataBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

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
	SgApp *pApp = pUpdateInfo->pApp;
	SgSwapchain* pSwapchain = &pUpdateInfo->pMaterialMap->swapchain;
	if(glfwWindowShouldClose(pApp->pWindow))
		return 0;
	glfwPollEvents();
	/* Retrieve Image */
    vkWaitForFences(pApp->device, 1, &pApp->pFrameFences[pApp->currentFrame], VK_TRUE, UINT64_MAX);

	VkResult res = vkAcquireNextImageKHR(pApp->device, pSwapchain->swapchain, UINT64_MAX, pApp->pFrameReadySemaphore[pApp->currentFrame], VK_NULL_HANDLE, &pApp->frameImageIndex);

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
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
		vkQueueWaitIdle(pApp->graphicsQueue);
		SgSwapchainCreateInfo swapchainCreateInfo = {
			.renderPass = pUpdateInfo->pMaterialMap->renderPass,
			.oldSwapchain = pUpdateInfo->pMaterialMap->swapchain.swapchain,
		};
		sgCleanupSwapchain(pApp, &pUpdateInfo->pMaterialMap->swapchain);
		sgCreateSwapchain(pApp, &swapchainCreateInfo, &pUpdateInfo->pMaterialMap->swapchain);
		SgUpdateCommandsInitInfo updateCommandsInitInfo = {
			.pMaterialMap = pUpdateInfo->pMaterialMap,
			.pMeshSet = pUpdateInfo->pMeshSet,
			.pResourceMap = pUpdateInfo->pUpdateCommands->pResourceMap,
		};
		sgInitUpdateCommands(&updateCommandsInitInfo, &pUpdateInfo->pUpdateCommands);
		return 1;
	} else if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
		sgLogError("[App Update]: Frame Image acquisition failure");
		return 0;
	}

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
		vkDestroySampler(pApp->device, pResource->imageSampler, VK_NULL_HANDLE);
		if (pResource->stagingBuffer.allocation) {
			vmaUnmapMemory(pApp->allocator, pResource->stagingBuffer.allocation);
			vmaDestroyBuffer(pApp->allocator, pResource->stagingBuffer.buffer, pResource->stagingBuffer.allocation);
		} else {
			vmaUnmapMemory(pApp->allocator, pResource->image.allocation);
		}
		vmaDestroyImage(pApp->allocator, pResource->image.image, pResource->image.allocation);
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
	free(pResource->pCommandBufferID);
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
	
	for (uint32_t i = 0; i < pApp->commandPoolCount; ++i) {
		vkResetCommandPool(pApp->device, pApp->pCommandPools[i], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
	for (uint32_t i = 0; i < SG_FRAME_QUEUE_LENGTH; ++i) {
		vkDestroySemaphore(pApp->device, pApp->pFrameReadySemaphore[i], VK_NULL_HANDLE);
		vkDestroySemaphore(pApp->device, pApp->pFrameFinishedSemaphore[i], VK_NULL_HANDLE);
		vkDestroyFence(pApp->device, pApp->pFrameFences[i], VK_NULL_HANDLE);
	}

	for (uint32_t i = 0; i < pApp->commandPoolCount; ++i) {
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
	free(pApp->pViewport);
	free(pApp->pScissor);
	free(pApp);
	ppApp = NULL;
}
