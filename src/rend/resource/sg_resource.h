#ifndef SAGE_RESORUCE_H
#define SAGE_RESORUCE_H 1
#include "hashmap.h"
#include "sg_base.h"

typedef struct SgResourceCreateInfo {
	SgResourceTypeFlags type;
	void* bytes;
	SgSize size;
	VkExtent3D extent;
	SgSize layerCount;
	SgSize levelCount;

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
                           const SgSize resourceCount,
                           const SgData** ppData,
                           const char** ppNames);
// Update resources with last assigned data (the pointer is stored, so you
// better not have destroyed the last used data)
SgResult sgUpdateAllResources(const SgApp* pApp, SgResourceMap* pResourceMap);

typedef struct SgBufferCreateInfo {
	void* bytes;
	SgSize size;
	SgResourceTypeFlags type;
} SgBufferCreateInfo;

void sgDestroyResource(const SgApp* pApp, SgResource** ppResource);

SgResult sgCreateBuffer(const SgApp* pApp,
                        SgBufferCreateInfo* pCreateInfo,
                        SgBufferData* ppBuffer);

typedef struct SgImageCreateInfo {
	VkExtent3D extent;
	VkFormat format;
	VkImageTiling tiling;
	VkImageLayout layout;
	VkImageType type;
	VkImageUsageFlags usage;
	VmaMemoryUsage memoryUsage;
	void* bytes;
	SgSize size;
	VkSampleCountFlags samples;
} SgImageCreateInfo;

SgResult sgCreateImage(const SgApp* pApp,
                       SgImageCreateInfo* pCreateInfo,
                       SgImageData* pImage);

typedef struct SgImageViewCreateInfo {
	SgImageData* pImage;
	VkImageViewType type;
	VkImageAspectFlags aspectFlags;
	SgSize levelCount;
	SgSize layerCount;
} SgImageViewCreateInfo;

SgResult sgCreateImageView(const SgApp* pApp,
                           const SgImageViewCreateInfo* pCreateInfo,
                           SgImageView* pImageView);

#endif
