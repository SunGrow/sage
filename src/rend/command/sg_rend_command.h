#ifndef SG_REND_COMMAND_H
#define SG_REND_COMMAND_H

#include "sg_base.h"

SgResult initCommandPools(SgApp *pApp);

typedef struct SgCommandBufferBeginEndInfo {
	const VkCommandPool *pCommandPool;
	VkCommandBufferLevel level;
	VkCommandBufferUsageFlags flags;
	VkFence *pFence;
	VkCommandBuffer* pCommandBuffer;
} SgCommandBufferBeginEndInfo;

SgResult sgBeginCommandBuffer(const SgApp* pApp, SgCommandBufferBeginEndInfo *pBeginEndInfo);
SgResult sgEndCommandBuffer(const SgApp* pApp, SgCommandBufferBeginEndInfo *pBeginEndInfo);
typedef struct SgImageTransferInfo {
	SgImage                    image;
	uint32_t                   srcQueueFamilyID;
	uint32_t                   dstQueueFamilyID;
	VkImageLayout              srcImageLayout;
	VkImageLayout              dstImageLayout;
    VkAccessFlags              srcAccessMask;
    VkAccessFlags              dstAccessMask;
    VkImageSubresourceRange    subresourceRange;
	VkPipelineStageFlags       srcStageMask;
	VkPipelineStageFlags       dstStageMask;
	VkDependencyFlags	       dependencyFlags;
} SgImageTransferInfo;
SgResult sgTransferImage(const SgApp* pApp, SgImageTransferInfo* pTransferInfo);
#endif
