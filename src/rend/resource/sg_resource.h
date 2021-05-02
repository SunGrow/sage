#ifndef SAGE_RESORUCE_H
#define SAGE_RESORUCE_H 1
#include "hashmap.h"
#include "sg_base.h"

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	void* bytes;
	uint32_t size;
	VkExtent3D extent;
	uint32_t layerCount;
	uint32_t levelCount;

	const char* pName;
} SgResourceCreateInfo;

SgResult sgCreateResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap);
SgResult sgAddResource(const SgApp* pApp,
                       const SgResourceCreateInfo* pCreateInfo,
                       SgResourceMap** ppResourceMap);
SgResult sgDestroyResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap);
SgResult sgInitResourceMap(const SgApp* pApp, SgResourceMap** ppResourceMap);

SgResult sgUpdateResource(const SgApp* pApp,
                          SgResourceMap* pResourceMap,
                          const SgData* pData,
                          const char* pName);
SgResult sgUpdateResources(const SgApp* pApp,
                           SgResourceMap* pResourceMap,
                           const uint32_t resourceCount,
                           const SgData** ppData,
                           const char** ppNames);
// Update resources with last assigned data (the pointer is stored, so you
// better not have destroyed the last used data)
SgResult sgUpdateAllResources(const SgApp* pApp, SgResourceMap* pResourceMap);

typedef struct SgBufferCreateInfo {
	void* bytes;
	uint32_t size;
	SgResourceTypeFlags type;
} SgBufferCreateInfo;

void sgDestroyResource(const SgApp* pApp, SgResource** ppResource);

SgResult sgCreateBuffer(const SgApp* pApp,
                        SgBufferCreateInfo* pCreateInfo,
                        SgBuffer* ppBuffer);

typedef struct SgImageCreateInfo {
	VkExtent3D extent;
	VkFormat format;
	VkImageTiling tiling;
	VkImageLayout layout;
	VkImageType type;
	VkImageUsageFlags usage;
	VmaMemoryUsage memoryUsage;
	void* bytes;
	uint32_t size;
	VkSampleCountFlags samples;
} SgImageCreateInfo;

SgResult sgCreateImage(const SgApp* pApp,
                       SgImageCreateInfo* pCreateInfo,
                       SgImage* pImage);

typedef struct SgImageViewCreateInfo {
	SgImage* pImage;
	VkImageViewType type;
	VkImageAspectFlags aspectFlags;
	uint32_t levelCount;
	uint32_t layerCount;
} SgImageViewCreateInfo;

SgResult sgCreateImageView(const SgApp* pApp,
                           const SgImageViewCreateInfo* pCreateInfo,
                           SgImageView* pImageView);

#endif
