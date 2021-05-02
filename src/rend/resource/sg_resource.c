#include <stdlib.h>
#include "log.h"
#include "sg_resource.h"
#include "command/sg_rend_command.h"
#include <string.h>


static int resourceKeyCompare(const void* a, const void* b, void* udata) {
	const SgResource* keyA = a;
	const SgResource* keyB = b;
	return strcmp(keyA->pName, keyB->pName);
}

static uint64_t resourceKeyHash(const void *item, uint64_t seed0, uint64_t seed1) {
    const SgResource* key = item;
    return hashmap_sip(key->pName, strlen(key->pName), seed0, seed1);
}

SgResult sgCreateResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap) {
	SgResourceMap* pResourceMap  = *ppResourceMap;
	SG_CALLOC_NUM(pResourceMap, 1);
	
	pResourceMap->pResourceMap = hashmap_new(sizeof(SgResource), 1, 0, 0, resourceKeyHash, resourceKeyCompare, NULL);
	VkFenceCreateInfo fenceCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};
	vkCreateFence(pApp->device, &fenceCreateInfo, VK_NULL_HANDLE, &pResourceMap->fence);
	VkCommandPoolCreateInfo commandPoolCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .queueFamilyIndex = pApp->graphicsQueueFamilyIdx,
	};
	VkResult vulkRes = vkCreateCommandPool(pApp->device, &commandPoolCreateInfo, VK_NULL_HANDLE, &pResourceMap->commandPool);
	if (vulkRes) {
		sgLogError("[Res]: Command Pool creation failure");
	}
	*ppResourceMap  = pResourceMap;
	return SG_SUCCESS;
}

static SgResult sgCreateResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResource *pResource) {
	pResource->type = pCreateInfo->type;
	pResource->dataBuffer.size = pCreateInfo->size;
	pResource->stagingBuffer.size = pCreateInfo->size;

	// Allocate resource buffer
	if (pCreateInfo->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		SgBufferCreateInfo stagingBufferCreateInfo = {
			.size  = pCreateInfo->size,
			.type  = SG_RESOURCE_TYPE_STAGING,
		};
		void *data;
		sgCreateBuffer(pApp, &stagingBufferCreateInfo, &pResource->stagingBuffer);
		vmaMapMemory(pApp->allocator,pResource->stagingBuffer.allocation, &data);
		memcpy(data, pCreateInfo->bytes, pCreateInfo->size);
		pResource->stagingBuffer.bytes = data;
		SgImageCreateInfo imageCreateInfo = {
			.bytes = pCreateInfo->bytes,
			.size  = pCreateInfo->size,
			.type  = VK_IMAGE_TYPE_2D,
			.layout = VK_IMAGE_LAYOUT_UNDEFINED,
			.format = VK_FORMAT_R8G8B8A8_SRGB,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.extent = pCreateInfo->extent,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		sgCreateImage(pApp, &imageCreateInfo, &pResource->image);

		SgImageView imageView;
		SgImageViewCreateInfo imageViewCreateInfo = {
			.pImage = &pResource->image,
			.type   = pCreateInfo->type,
			.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = pCreateInfo->levelCount,
			.layerCount = pCreateInfo->layerCount,
		};
		sgCreateImageView(pApp, &imageViewCreateInfo, &imageView);
		pResource->imageView = imageView.imageView;
		// TODO: in a function
		VkSamplerCreateInfo samplerInfo = {
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.anisotropyEnable = VK_FALSE,
			.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		};
		vkCreateSampler(pApp->device, &samplerInfo, VK_NULL_HANDLE, &pResource->imageSampler);
		/* Transition image */
		{
			SgImageTransferInfo transferInfo = {
				.image = pResource->image,
				.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.layerCount = 1,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			};
			sgTransferImage(pApp, &transferInfo);
		}
		
		{
			SgImageTransferInfo transferInfo = {
				.image = pResource->image,
				.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.layerCount = 1,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			};
			sgTransferImage(pApp, &transferInfo);
		}
		
	} else {
		SgBufferCreateInfo bufferCreateInfo = {
			.size  = pCreateInfo->size,
			.type  = pCreateInfo->type,
		};
		SgBufferCreateInfo stagingBufferCreateInfo = {
			.size  = pCreateInfo->size,
			.type  = SG_RESOURCE_TYPE_STAGING,
		};
		void *data;
		if (SG_RESOURCE_TYPE_REQIRE_STAGING_MASK & pCreateInfo->type) {
			sgCreateBuffer(pApp, &bufferCreateInfo, &pResource->dataBuffer);
			sgCreateBuffer(pApp, &stagingBufferCreateInfo, &pResource->stagingBuffer);
			vmaMapMemory(pApp->allocator,pResource->stagingBuffer.allocation, &data);
			pResource->stagingBuffer.bytes = data;
			memcpy(pResource->stagingBuffer.bytes, pCreateInfo->bytes, pCreateInfo->size);
		} else {
			sgCreateBuffer(pApp, &bufferCreateInfo, &pResource->dataBuffer);
			vmaMapMemory(pApp->allocator,pResource->dataBuffer.allocation, &data);
			pResource->dataBuffer.bytes = data;
			memcpy(pResource->dataBuffer.bytes, pCreateInfo->bytes, pCreateInfo->size);
		}
	}
	SG_CALLOC_NUM(pResource->pCommandBufferID, 1);
	return SG_SUCCESS;
}

SgResult sgAddResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResourceMap** ppResourceMap) {
	SgResourceMap* pResourceMap  = *ppResourceMap;
	SgResource resource = {
		.pName = pCreateInfo->pName,
	};
	SgResource* pResource = hashmap_get(pResourceMap->pResourceMap, &resource);
	if (pResource) {
		sgLogDebug_Debug("Resource under the name \"%s\" is already in a map", pCreateInfo->pName);
		return -1;
	}
	sgCreateResource(pApp, pCreateInfo, &resource);
	resource.lastBytes = pCreateInfo->bytes;
	resource.lastSize = pCreateInfo->size;
	hashmap_set(pResourceMap->pResourceMap, &resource);
	
	
	*ppResourceMap  = pResourceMap;

	return SG_SUCCESS;
}

struct SgResourceIterInfo {
	const SgApp* pApp;
	SgResourceMap* pResourceMap;
	uint32_t iter;
};

_Bool sgImagesToTransferResourcesIter(const void* item, void* data) {
	const SgResource* pResource = item;
	struct SgResourceIterInfo* iterInfo = data;
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		SgImageTransferInfo transferInfo = {
			.image = pResource->image,
			.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		};
		sgTransferImage(iterInfo->pApp, &transferInfo);
	}
	return 1;
}

_Bool sgImagesToRenderResourcesIter(const void* item, void* data) {
	const SgResource* pResource = item;
	struct SgResourceIterInfo* iterInfo = data;
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		SgImageTransferInfo transferInfo = {
			.image = pResource->image,
			.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		};
		sgTransferImage(iterInfo->pApp, &transferInfo);
	}
	return 1;
}

_Bool sgInitResourcesIter(const void* item, void* data) {
	const SgResource* pResource = item;
	struct SgResourceIterInfo* iterInfo = data;
	*pResource->pCommandBufferID = iterInfo->iter;
	VkCommandBuffer* pCommandBuffer = &iterInfo->pResourceMap->pCommadBuffers[iterInfo->iter];

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	vkBeginCommandBuffer(*pCommandBuffer, &beginInfo);
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		VkBufferImageCopy region = {
			.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.imageSubresource.layerCount = 1,
			.imageExtent = pResource->image.extent,
		};

	    vkCmdCopyBufferToImage (
	        *pCommandBuffer,
	        pResource->stagingBuffer.buffer,
	        pResource->image.image,
	        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	        1,
	        &region
	    );
	} else if (pResource->type & SG_RESOURCE_TYPE_REQIRE_STAGING_MASK) {
		VkBufferCopy copyRegion = {
			.size = pResource->stagingBuffer.size,
		};
		vkCmdCopyBuffer(*pCommandBuffer, pResource->stagingBuffer.buffer, pResource->dataBuffer.buffer, 1, &copyRegion);
	}

	vkEndCommandBuffer(*pCommandBuffer);

	++iterInfo->iter;
	return 1;
}

SgResult sgInitResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap) {
	SgResourceMap* pResourceMap = *ppResourceMap;

	uint32_t resourceCount = hashmap_count(pResourceMap->pResourceMap);


	SG_CALLOC_NUM(pResourceMap->pCommadBuffers, resourceCount);
	SG_CALLOC_NUM(pResourceMap->pCommadBuffersForSpecificUpdate, resourceCount);

	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = pResourceMap->commandPool,
		.commandBufferCount = resourceCount,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	};
	vkAllocateCommandBuffers(pApp->device, &allocInfo, pResourceMap->pCommadBuffers);
	struct SgResourceIterInfo iterInfo = {
		.pResourceMap = pResourceMap,
		.pApp = pApp,
		.iter = 0
	};
	hashmap_scan(pResourceMap->pResourceMap, sgInitResourcesIter, &iterInfo);

	*ppResourceMap = pResourceMap;
	return SG_SUCCESS;
};

_Bool sgDestroyResourceIter(const void* item, void* data) {
	SgResource* pResource = item;
	SgApp* pApp = data;
	sgDestroyResource(pApp, &pResource);
	return 1;
}

SgResult sgDestroyResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap) {
	SgResourceMap* pResourceMap = *ppResourceMap;
	hashmap_scan(pResourceMap->pResourceMap, sgDestroyResourceIter, pApp);
	hashmap_free(pResourceMap->pResourceMap);

	vkDestroyCommandPool(pApp->device, pResourceMap->commandPool, VK_NULL_HANDLE);
	free(pResourceMap->pCommadBuffers);
	free(pResourceMap->pCommadBuffersForSpecificUpdate);
	vkDestroyFence(pApp->device, pResourceMap->fence, VK_NULL_HANDLE);
	free(pResourceMap);
	*ppResourceMap = pResourceMap;
	return SG_SUCCESS;
}


SgResult sgCreateBuffer(const SgApp* pApp, SgBufferCreateInfo* pCreateInfo, SgBuffer* pBuffer) {
	VkBufferCreateInfo bufferCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = pCreateInfo->size,
	};
	pBuffer->size = pCreateInfo->size;
	VmaAllocationCreateInfo allocationInfo = {0};
	switch (pCreateInfo->type) {
		case (SG_RESOURCE_TYPE_STAGING):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT; 
			break;
		case (SG_RESOURCE_TYPE_MESH):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case (SG_RESOURCE_TYPE_UNIFORM):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			allocationInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			allocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_CACHED_BIT; 
			break;
		case (SG_RESOURCE_TYPE_INDICES):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
	}

	VkResult res = vmaCreateBuffer(pApp->allocator, &bufferCreateInfo, &allocationInfo, &pBuffer->buffer, &pBuffer->allocation, NULL);
	if (res) {
		log_warn("[Res]: Buffer creation error");
	}
	return SG_SUCCESS;
}

SgResult sgCreateImage(const SgApp* pApp, SgImageCreateInfo* pCreateInfo, SgImage* pImage) {
	VkImageCreateInfo createinfo = {
	    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
	    .usage = pCreateInfo->usage,
	    .imageType = VK_IMAGE_TYPE_2D,
	    .format = pCreateInfo->format,
	    .extent = pCreateInfo->extent,
	    .tiling = pCreateInfo->tiling,
	    .flags = 0,
	    .mipLevels = 1,
	    .initialLayout = pCreateInfo->layout,
	    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	    .arrayLayers = 1,
	    .samples = pCreateInfo->samples,
	};

	VmaAllocationCreateInfo allocationInfo = {
		.usage = pCreateInfo->memoryUsage,
	};

	vmaCreateImage(pApp->allocator, &createinfo, &allocationInfo, &pImage->image, &pImage->allocation, NULL);
	pImage->size   = pCreateInfo->size;
	pImage->bytes  = pCreateInfo->bytes;
	pImage->format = pCreateInfo->format;
	pImage->extent = pCreateInfo->extent;
	return SG_SUCCESS;
}

SgResult sgCreateImageView(const SgApp* pApp, const SgImageViewCreateInfo *pCreateInfo, SgImageView *pImageView) {
	VkImageViewCreateInfo createinfo = {
	    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    .image = pCreateInfo->pImage->image,
	    .viewType = pCreateInfo->type,
	    .format = pCreateInfo->pImage->format,
	    .subresourceRange.aspectMask = pCreateInfo->aspectFlags,
	    .subresourceRange.levelCount = pCreateInfo->levelCount,
	    .subresourceRange.layerCount = pCreateInfo->layerCount,
	};
	vkCreateImageView(pApp->device, &createinfo, VK_NULL_HANDLE, &pImageView->imageView);
	return SG_SUCCESS;
}


// Add image
SgResult sgUpdateResource(const SgApp* pApp, SgResourceMap* pResourceMap, const SgData* pData, const char* pName) {

	SgResource findResource = {
		.pName = pName,
	};
	SgResource* pResource = hashmap_get(pResourceMap->pResourceMap, &findResource);
	if (pResource == NULL) {
		sgLogDebug_Debug("Resource under the name %s has not been found", pName);
		return -1;
	}

    if (pData->size > pResource->dataBuffer.size) {
        sgLogWarn("[Res]: Data size exceeds resource size. Possible memory corruption");
    }
    if (SG_RESOURCE_TYPE_REQIRE_STAGING_MASK & pResource->type 
			|| pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		memcpy(pResource->stagingBuffer.bytes, pData->bytes, pData->size);
    } else {
        memcpy(pResource->dataBuffer.bytes, pData->bytes, pData->size);
        return SG_SUCCESS;
	}

	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		SgImageTransferInfo transferInfo = {
			.image = pResource->image,
			.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		};
		sgTransferImage(pApp, &transferInfo);
	}

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &pResourceMap->pCommadBuffers[*pResource->pCommandBufferID],
	};
	
	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pResourceMap->fence);
	vkWaitForFences(pApp->device, 1, &pResourceMap->fence, VK_TRUE, 0xFFFFFFFF);
	vkResetFences(pApp->device, 1, &pResourceMap->fence);
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		SgImageTransferInfo transferInfo = {
			.image = pResource->image,
			.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
			.subresourceRange.baseArrayLayer = 0,
			.srcAccessMask = 0,
			.dstAccessMask = 0,
			.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		};
		sgTransferImage(pApp, &transferInfo);
	}

	return SG_SUCCESS;
}

SgResult sgMemcpyToResource(const SgData* pData, const SgResource* pResource) {
	// TODO: Size safety
    if (pResource->lastSize > pResource->dataBuffer.size) {
        sgLogWarn("[Res]: Data size exceeds resource size. Possible memory corruption");
    }
    if (SG_RESOURCE_TYPE_REQIRE_STAGING_MASK & pResource->type 
			|| pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		memcpy(pResource->stagingBuffer.bytes, pData->bytes, pData->size);
    } else {
        memcpy(pResource->dataBuffer.bytes, pData->bytes, pData->size);
        return SG_SUCCESS;
	}

	return 1;
}

_Bool sgUpdateResourceIter(const void* item, void* data) {
	const SgResource* pResource = item;
	SgData resData = {
		.bytes = pResource->lastBytes,
		.size = pResource->lastSize,
	};
	return sgMemcpyToResource(&resData, pResource);
}

SgResult sgUpdateResources(const SgApp* pApp, SgResourceMap* pResourceMap, const uint32_t resourceCount, const SgData** ppData, const char** ppNames) {
	for (uint32_t i = 0; i < resourceCount; ++i) {
		SgResource findResource = {
			.pName = ppNames[i],
		};
		SgResource* pResource = hashmap_get(pResourceMap->pResourceMap, &findResource);
		if (pResource==NULL) {
			sgLogDebug_Debug("Resource under the name %s not found", ppNames[i]);
			continue;
		}
		SgData resData;
		if (ppData[i] == NULL) {
			resData.bytes = pResource->lastBytes;
			resData.size  = pResource->lastSize;
		} else {
			resData.bytes = ppData[i]->bytes;
			resData.size  = ppData[i]->size;
		}
		sgMemcpyToResource(&resData, pResource);
		pResourceMap->pCommadBuffersForSpecificUpdate[i] = pResourceMap->pCommadBuffers[*pResource->pCommandBufferID];

		if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
			SgImageTransferInfo transferInfo = {
				.image = pResource->image,
				.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.layerCount = 1,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			};
			sgTransferImage(pApp, &transferInfo);
		}

	}
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = resourceCount,
		.pCommandBuffers = pResourceMap->pCommadBuffersForSpecificUpdate,
	};
	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pResourceMap->fence);
	vkWaitForFences(pApp->device, 1, &pResourceMap->fence, VK_TRUE, 0xFFFFFFFF);
	vkResetFences(pApp->device, 1, &pResourceMap->fence);
	for (uint32_t i = 0; i < resourceCount; ++i) {
		SgResource findResource = {
			.pName = ppNames[i],
		};
		SgResource* pResource = hashmap_get(pResourceMap->pResourceMap, &findResource);
		if (pResource==NULL) {
			sgLogDebug_Debug("Resource under the name %s not found", ppNames[i]);
			continue;
		}
		if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
			SgImageTransferInfo transferInfo = {
				.image = pResource->image,
				.srcImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.dstImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.srcQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.dstQueueFamilyID = VK_QUEUE_FAMILY_IGNORED,
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.baseMipLevel = 0,
				.subresourceRange.layerCount = 1,
				.subresourceRange.levelCount = 1,
				.subresourceRange.baseArrayLayer = 0,
				.srcAccessMask = 0,
				.dstAccessMask = 0,
				.srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			};
			sgTransferImage(pApp, &transferInfo);
		}
	}
	return SG_SUCCESS;
}

SgResult sgUpdateAllResources(const SgApp* pApp, SgResourceMap* pResourceMap) {
	struct SgResourceIterInfo iterInfo = {
		.pResourceMap = pResourceMap,
		.pApp = pApp,
		.iter = 0
	};
	hashmap_scan(pResourceMap->pResourceMap, sgImagesToTransferResourcesIter, &iterInfo);
	hashmap_scan(pResourceMap->pResourceMap, sgUpdateResourceIter, NULL);
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = hashmap_count(pResourceMap->pResourceMap),
		.pCommandBuffers = pResourceMap->pCommadBuffers,
	};
	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, pResourceMap->fence);
	vkWaitForFences(pApp->device, 1, &pResourceMap->fence, VK_TRUE, 0xFFFFFFFF);
	vkResetFences(pApp->device, 1, &pResourceMap->fence);
	hashmap_scan(pResourceMap->pResourceMap, sgImagesToRenderResourcesIter, &iterInfo);

	return SG_SUCCESS;
}
