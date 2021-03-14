#include <stdlib.h>
#include "log.h"
#include "sg_resource.h"
#include <string.h>


SgResult sgCreateResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResource **ppResource) {
	SgResource *pResource = calloc(1, sizeof(pResource[0]));
	pResource->stage = pCreateInfo->stage;
	pResource->type = pCreateInfo->type;
	pResource->binding = pCreateInfo->binding;
	pResource->dataBuffer.size = pCreateInfo->size;
	pResource->stagingBuffer.size = pCreateInfo->size;
	// Allocate transfer command buffer
	VkCommandPoolCreateInfo commandPoolCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	    .queueFamilyIndex = pApp->graphicsQueueFamilyIdx,
	};
	VkResult vulkRes = vkCreateCommandPool(pApp->device, &commandPoolCreateInfo, VK_NULL_HANDLE, &pResource->commandPool);
	if (vulkRes) {
		log_warn("[Res]: Command Pool creation failure");
	}

	VkCommandBufferAllocateInfo commandAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    	.commandPool = pResource->commandPool,
    	.commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(pApp->device, &commandAllocInfo, &pResource->commandBuffer);

	// Allocate resource buffer
	if (pCreateInfo->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
     /* TODO: texture type */
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
		SgData dataToCopyFrom = {
			.bytes = pCreateInfo->bytes,
			.size = pCreateInfo->size,
		};
		sgUpdateResource(pApp, &dataToCopyFrom, &pResource);
	}

	*ppResource = pResource;

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
	return SG_SUCCESS;
}

SgResult sgCreateImageView(const SgApp* pApp, const SgImageViewCreateInfo *pCreateInfo, SgImageView *pImageView) {
	VkImageViewCreateInfo createinfo = {
	    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
	    .image = pCreateInfo->pImage->image,
	    .viewType = pCreateInfo->type,
	    .format = pCreateInfo->pImage->format,
	    .subresourceRange.aspectMask = pCreateInfo->aspectFlags,
	    .subresourceRange.levelCount = 1,
	    .subresourceRange.layerCount = 1,
	};
	vkCreateImageView(pApp->device, &createinfo, VK_NULL_HANDLE, &pImageView->imageView);
	return SG_SUCCESS;
}


// TODO: Confusing API. Fix when I'm less angry at life.
// Add image
SgResult sgUpdateResource(const SgApp* pApp, const SgData* pData, SgResource** ppResource) {
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	SgResource* pResource = *ppResource;
	// TODO: Image Type
	if (pResource->type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		log_info("[Res]: Image Type is not yet implemented");
		return SG_SUCCESS;
	}

	// Goodness gracious, this SG_RESOURCE_TYPE thing sure is 
	// getting hard to maintain...

	// TODO: Size safety
	if (pData->size > pResource->dataBuffer.size) {
		log_warn("[Res]: Data size exceeds resource size. Possible memory corruption");
	}
	if (SG_RESOURCE_TYPE_REQIRE_STAGING_MASK & pResource->type) {
		memcpy(pResource->stagingBuffer.bytes, pData->bytes, pData->size);
	} else {
		memcpy(pResource->dataBuffer.bytes, pData->bytes, pData->size);
		return SG_SUCCESS;
	}

	// From staging buffer to data
	vkBeginCommandBuffer(pResource->commandBuffer, &beginInfo);

	VkBufferCopy copyRegion = {
		.size = pResource->stagingBuffer.size,
	};
	vkCmdCopyBuffer(pResource->commandBuffer, pResource->stagingBuffer.buffer, pResource->dataBuffer.buffer, 1, &copyRegion);

	vkEndCommandBuffer(pResource->commandBuffer);
	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &pResource->commandBuffer,
	};
	
	vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(pApp->graphicsQueue); // TMP
//	vkFreeCommandBuffers(pApp->device, pResource->commandPool, 1, &pResource->commandBuffer);
	vkResetCommandPool(pApp->device, pResource->commandPool, 0);
	*ppResource = pResource;
	return SG_SUCCESS;
}
