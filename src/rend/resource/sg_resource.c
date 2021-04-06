#include <stdlib.h>
#include "log.h"
#include "sg_resource.h"
#include "command/sg_rend_command.h"
#include <string.h>


SgResult sgCreateResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResource **ppResource) {
	SgResource *pResource;
	SG_CALLOC_NUM(pResource, 1);
	pResource->resourceBinding.stage = pCreateInfo->stage;
	pResource->resourceBinding.type = pCreateInfo->type;
	pResource->resourceBinding.binding = pCreateInfo->binding;
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
		SgData dataToCopyFrom = {
			.bytes = pCreateInfo->bytes,
			.size = pCreateInfo->size,
		};
		sgUpdateResource(pApp, &dataToCopyFrom, &pResource);
		/* Transition image */
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
		/**/
		//
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
	if (pResource->resourceBinding.type & SG_RESOURCE_TYPE_IS_IMAGE_MASK) {
		memcpy(pResource->stagingBuffer.bytes, pData->bytes, pData->size);
		// From staging buffer to data
		vkBeginCommandBuffer(pResource->commandBuffer, &beginInfo);
		VkBufferImageCopy region = {
			.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.imageSubresource.layerCount = 1,
			.imageExtent = pResource->image.extent,
		};

	    vkCmdCopyBufferToImage (
	        pResource->commandBuffer,
	        pResource->stagingBuffer.buffer,
	        pResource->image.image,
	        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	        1,
	        &region
	    );

		vkEndCommandBuffer(pResource->commandBuffer);
		VkSubmitInfo submitInfo = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers = &pResource->commandBuffer,
		};
		
		vkQueueSubmit(pApp->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(pApp->graphicsQueue); // TMP
		vkResetCommandPool(pApp->device, pResource->commandPool, 0);
		*ppResource = pResource;
		return SG_SUCCESS;
	}

	// TODO: Size safety
    if (pData->size > pResource->dataBuffer.size) {
        log_warn("[Res]: Data size exceeds resource size. Possible memory corruption");
    }
    if (SG_RESOURCE_TYPE_REQIRE_STAGING_MASK & pResource->resourceBinding.type) {
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
	vkResetCommandPool(pApp->device, pResource->commandPool, 0);
	*ppResource = pResource;
	return SG_SUCCESS;
}
