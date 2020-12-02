#ifndef SAGE_RESORUCE_H
#define SAGE_RESORUCE_H 1
#include "sage_base.h"

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	SgShaderStageFlags  stage;
	void*               bytes;
	uint32_t            size;
	uint32_t            binding;
} SgResourceCreateInfo;

SgResult sgCreateResource(const SgApp* pApp, const SgResourceCreateInfo *pCreateInfo, SgResource **ppResource);

SgResult sgUpdateResource(const SgApp* pApp, SgResource** ppResource);

typedef struct SgBufferCreateInfo {
	void*               bytes;
	uint32_t            size;
	SgResourceTypeFlags type;
} SgBufferCreateInfo;

SgResult sgCreateBuffer(const SgApp* pApp, SgBufferCreateInfo* pCreateInfo, SgBuffer* ppBuffer);

typedef struct SgImageCreateInfo {
	VkExtent3D          extent;
	VkFormat            format;
	VkImageTiling       tiling;
	VkImageLayout       layout;
	VkImageType         type;
	VkImageUsageFlags   usage;
	VmaMemoryUsage      memoryUsage;
	void*               bytes;
	uint32_t            size;
} SgImageCreateInfo;

SgResult sgCreateImage(const SgApp* pApp, SgImageCreateInfo* pCreateInfo, SgImage* pImage);

typedef struct SgImageViewCreateInfo {
	SgImage*               pImage;
	VkImageViewType        type;
	VkImageAspectFlags     aspectFlags;
} SgImageViewCreateInfo;

SgResult sgCreateImageView(const SgApp* pApp, const SgImageViewCreateInfo *pCreateInfo, SgImageView *pImageView);

#endif
