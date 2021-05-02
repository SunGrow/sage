#ifndef SG_REND_DEVICE_H
#define SG_REND_DEVICE_H

#define VK_NO_PROTOTYPES
#include "sg_base.h"
#include "volk.h"

typedef struct SgPhysicalDeviceGetInfo {
	VkInstance* pInstance;
	VkSurfaceKHR* pSurface;
} SgPhysicalDeviceGetInfo;
SgResult getPhysicalDevice(SgPhysicalDeviceGetInfo* pGetInfo,
                           VkPhysicalDevice* pPhysicalDevice);
uint32_t getGraphicsFamilyIndex(VkPhysicalDevice pd);
typedef struct SgDeviceQueueCreateInfo {
	float* pQueuePriorities;
	uint32_t queueIndex;
	uint32_t queueCount;
} SgDeviceQueueCreateInfo;
typedef struct SgLogicalDeviceGetInfo {
	SgDeviceQueueCreateInfo* pQueueCreateInfos;
	uint32_t createInfosCount;
	VkPhysicalDevice physicalDevice;
} SgLogicalDeviceGetInfo;
SgResult getLogicalDevice(SgLogicalDeviceGetInfo* pGetInfo, VkDevice* pDevice);

#endif
