#ifndef SG_REND_COMMAND_H
#define SG_REND_COMMAND_H

#include "sg_base.h"

typedef struct SgCommandPoolsCreateInfo {
} SgCommandPoolsCreateInfo;

SgResult createCommandPools(const VkDevice* pDevice,
                            const SgSize    queueFamilyID,
                            pthread_t*      pThreads,
                            VkCommandPool*  pCommandPools,
                            SgSize          commandPoolCount);

typedef struct SgCommandBufferBeginEndInfo {
	const VkCommandPool*      pCommandPool;
	VkCommandBufferLevel      level;
	VkCommandBufferUsageFlags flags;
	VkFence*                  pFence;
	VkCommandBuffer*          pCommandBuffer;
} SgCommandBufferBeginEndInfo;

SgResult sgBeginCommandBuffer(const SgApp*                 pApp,
                              SgCommandBufferBeginEndInfo* pBeginEndInfo);
SgResult sgEndCommandBuffer(const SgApp*                 pApp,
                            SgCommandBufferBeginEndInfo* pBeginEndInfo);
typedef struct SgImageTransferInfo {
	SgImageData             image;
	SgSize                  srcQueueFamilyID;
	SgSize                  dstQueueFamilyID;
	VkImageLayout           srcImageLayout;
	VkImageLayout           dstImageLayout;
	VkAccessFlags           srcAccessMask;
	VkAccessFlags           dstAccessMask;
	VkImageSubresourceRange subresourceRange;
	VkPipelineStageFlags    srcStageMask;
	VkPipelineStageFlags    dstStageMask;
	VkDependencyFlags       dependencyFlags;
} SgImageTransferInfo;
SgResult sgTransferImage(const SgApp* pApp, SgImageTransferInfo* pTransferInfo);
#endif
