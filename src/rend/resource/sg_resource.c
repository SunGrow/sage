#include <stdlib.h>
#include "log.h"
#include "sg_resource.h"


SgResult sgCreateResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResource **ppResource) {
	SgResource *pResource = calloc(1, sizeof(pResource[0]));
	pResource->stage = pCreateInfo->stage;
	pResource->type = pCreateInfo->type;
	pResource->binding = pCreateInfo->binding;
	pResource->dataBuffer.bytes = pCreateInfo->bytes;
	pResource->dataBuffer.size = pCreateInfo->size;
	pResource->stagingBuffer.bytes = pCreateInfo->bytes;
	pResource->stagingBuffer.size = pCreateInfo->size;
	// Allocate transfer command buffer
	VkCommandBufferAllocateInfo commandAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    	.commandPool = pApp->pCommandPools[0], // TODO: Use dedicated command pools
    	.commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(pApp->device, &commandAllocInfo, &pResource->commandBuffer);

	// Allocate resource buffer
	if (pCreateInfo->type == SG_RESOURCE_TYPE_TEXTURE_2D) {
     /* TODO: texture type */
	} else {
		SgBufferCreateInfo bufferCreateInfo = {
			.bytes = pCreateInfo->bytes,
			.size  = pCreateInfo->size,
			.type  = pCreateInfo->type,
		};
		SgBufferCreateInfo stagingBufferCreateInfo = {
			.bytes = pCreateInfo->bytes,
			.size  = pCreateInfo->size,
			.type  = SG_RESOURCE_TYPE_STAGING,
		};
		switch (pCreateInfo->type) {
		case (SG_RESOURCE_TYPE_MESH):
			sgCreateBuffer(pApp, &bufferCreateInfo, &pResource->dataBuffer);
			sgCreateBuffer(pApp, &stagingBufferCreateInfo, &pResource->stagingBuffer);
			vmaMapMemory(pApp->allocator,pResource->stagingBuffer.allocation, pResource->stagingBuffer.bytes);
			break;
		case (SG_RESOURCE_TYPE_UNIFORM):
			sgCreateBuffer(pApp, &bufferCreateInfo, &pResource->dataBuffer);
			vmaMapMemory(pApp->allocator,pResource->dataBuffer.allocation, pResource->dataBuffer.bytes);
			break;
		case (SG_RESOURCE_TYPE_INDICES):
			sgCreateBuffer(pApp, &bufferCreateInfo, &pResource->dataBuffer);
			sgCreateBuffer(pApp, &stagingBufferCreateInfo, &pResource->stagingBuffer);
			vmaMapMemory(pApp->allocator,pResource->stagingBuffer.allocation, pResource->stagingBuffer.bytes);
			break;
		}
	}

	*ppResource = pResource;

	return SG_SUCCESS;
}

SgResult sgCreateBuffer(const SgApp* pApp, SgBufferCreateInfo* pCreateInfo, SgBuffer* pBuffer) {
	VkBufferCreateInfo bufferCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = pCreateInfo->size,
	};
	VmaAllocationCreateInfo allocationInfo = {0};
	switch (pCreateInfo->type) {
		case (SG_RESOURCE_TYPE_STAGING):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			break;
		case (SG_RESOURCE_TYPE_MESH):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
			break;
		case (SG_RESOURCE_TYPE_UNIFORM):
			bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
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
	pBuffer->size = pCreateInfo->size;
	pBuffer->bytes = pCreateInfo->bytes;
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
	    .samples = VK_SAMPLE_COUNT_1_BIT,
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
SgResult sgUpdateResource(const SgApp* pApp, SgResource** ppResource) {
	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	SgResource* pResource = *ppResource;

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
	vkQueueWaitIdle(pApp->graphicsQueue);
	*ppResource = pResource;
	return SG_SUCCESS;
}
