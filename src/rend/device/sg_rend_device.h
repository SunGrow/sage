#ifndef SG_REND_DEVICE_H
#define SG_REND_DEVICE_H

#define VK_NO_PROTOTYPES
#include "sg_base.h"
#include "volk.h"

typedef struct SgPhysicalDeviceGetInfo {
	VkInstance*   pInstance;
	VkSurfaceKHR* pSurface;
} SgPhysicalDeviceGetInfo;
SgResult getPhysicalDevice(SgPhysicalDeviceGetInfo* pGetInfo,
                           VkPhysicalDevice*        pPhysicalDevice);
SgSize   getGraphicsFamilyIndex(VkPhysicalDevice pd);
typedef struct SgDeviceQueueCreateInfo {
	float* pQueuePriorities;
	SgSize queueIndex;
	SgSize queueCount;
} SgDeviceQueueCreateInfo;
typedef struct SgLogicalDeviceGetInfo {
	SgDeviceQueueCreateInfo* pQueueCreateInfos;
	SgSize                   createInfosCount;
	VkPhysicalDevice         physicalDevice;
} SgLogicalDeviceGetInfo;
SgResult getLogicalDevice(SgLogicalDeviceGetInfo* pGetInfo, VkDevice* pDevice);

#endif
