#include "sg_rend_command.h"

typedef struct ThreadCommandPoolCreateInfo {
	const VkDevice* pDevice;
	SgSize          queueFamilyID;
	VkCommandPool*  pCommandPools;
	SgSize          commandPoolCount;
	SgSize          threadIndex;
} ThreadCommandPoolCreateInfo;

static void* createCommandPool(void* threadCommandPoolInfo) {
	VkResult                     vulkRes;
	VkCommandPool                commandpool = 0;
	ThreadCommandPoolCreateInfo* info =
	    (ThreadCommandPoolCreateInfo*)threadCommandPoolInfo;
	SgSize index = info->threadIndex;

	VkCommandPoolCreateInfo createInfo = {
	    .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	    .queueFamilyIndex = info->queueFamilyID,
	};

	vulkRes = vkCreateCommandPool(*info->pDevice, &createInfo, VK_NULL_HANDLE,
	                              &commandpool);
	if (vulkRes) {
		sgLogWarn("[AppInit]: Command Pool creation failure %d", index);
		return (void*)-1;
	}
	info->pCommandPools[index] = commandpool;
	return (void*)SG_SUCCESS;
}

SgResult createCommandPools(const VkDevice* pDevice,
                            const SgSize    queueFamilyID,
                            pthread_t*      pThreads,
                            VkCommandPool*  pCommandPools,
                            SgSize          commandPoolCount) {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// Submit pool create commands
	for (SgSize i = 0; i < commandPoolCount; ++i) {
		ThreadCommandPoolCreateInfo info = {
		    .commandPoolCount = commandPoolCount,
		    .pCommandPools    = pCommandPools,
		    .pDevice          = pDevice,
		    .queueFamilyID    = queueFamilyID,
		    .threadIndex      = i,
		};
		pthread_create(&pThreads[i], &attr, createCommandPool, &info);
		int rc = pthread_join(pThreads[i], NULL);
		if (rc) {
			sgLogError("[AppInit]: Return code from pthread_join() is %d\n", rc);
		}
	}
	for (SgSize i = 0; i < commandPoolCount; ++i) {
		if (pCommandPools[i] == VK_NULL_HANDLE) {
			sgLogWarn("[AppInit]: CommandPool %d not initialized", i);
		}
	}
	sgLogInfo_Debug("[AppInit]: Command Pools initialization completed");
	return SG_SUCCESS;
}

SgResult sgBeginCommandBuffer(const SgApp*                 pApp,
                              SgCommandBufferBeginEndInfo* pBeginEndInfo) {
	VkCommandBufferAllocateInfo allocInfo = {
	    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandPool        = *pBeginEndInfo->pCommandPool,
	    .commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(pApp->device, &allocInfo,
	                         pBeginEndInfo->pCommandBuffer);

	VkCommandBufferBeginInfo beginInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(*pBeginEndInfo->pCommandBuffer, &beginInfo);

	return SG_SUCCESS;
}
SgResult sgEndCommandBuffer(const SgApp*                 pApp,
                            SgCommandBufferBeginEndInfo* pBeginEndInfo) {
	vkEndCommandBuffer(*pBeginEndInfo->pCommandBuffer);

	VkSubmitInfo submitInfo = {
	    .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .commandBufferCount = 1,
	    .pCommandBuffers    = pBeginEndInfo->pCommandBuffer,
	};

	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, *pBeginEndInfo->pFence);
	vkWaitForFences(pApp->device, 1, pBeginEndInfo->pFence, true, UINT64_MAX);
	vkResetFences(pApp->device, 1, pBeginEndInfo->pFence);
	vkResetCommandPool(pApp->device, pApp->pCommandPools[2], 0);

	return SG_SUCCESS;
}

SgResult sgTransferImage(const SgApp*         pApp,
                         SgImageTransferInfo* pTransferInfo) {
	VkFence           fence;
	VkFenceCreateInfo fenceCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};
	vkCreateFence(pApp->device, &fenceCreateInfo, VK_NULL_HANDLE, &fence);
	/* Begin command buffer */
	VkCommandBuffer             commandBuffer;
	SgCommandBufferBeginEndInfo beginEndInfo = {
	    .pCommandPool   = &pApp->pCommandPools[2],
	    .level          = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .flags          = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	    .pFence         = &fence,
	    .pCommandBuffer = &commandBuffer,
	};

	sgBeginCommandBuffer(pApp, &beginEndInfo);
	/**/

	VkImageMemoryBarrier barrier = {
	    .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
	    .oldLayout           = pTransferInfo->srcImageLayout,
	    .newLayout           = pTransferInfo->dstImageLayout,
	    .srcQueueFamilyIndex = pTransferInfo->srcQueueFamilyID,
	    .dstQueueFamilyIndex = pTransferInfo->dstQueueFamilyID,
	    .image               = pTransferInfo->image.image,
	    .subresourceRange    = pTransferInfo->subresourceRange,
	    .srcAccessMask       = pTransferInfo->srcAccessMask,
	    .dstAccessMask       = pTransferInfo->dstAccessMask,
	};

	vkCmdPipelineBarrier(
	    commandBuffer, pTransferInfo->srcStageMask, pTransferInfo->dstStageMask,
	    pTransferInfo->dependencyFlags, 0, NULL, 0, NULL, 1, &barrier);

	/* End command buffer */
	sgEndCommandBuffer(pApp, &beginEndInfo);
	vkDestroyFence(pApp->device, fence, VK_NULL_HANDLE);
	/**/

	return SG_SUCCESS;
}
