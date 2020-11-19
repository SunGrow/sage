#ifndef SG_REND_DEVICE_H
#define SG_REND_DEVICE_H

#define VK_NO_PROTOTYPES
#include "volk.h"

VkPhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice *pPhysicalDevices, uint32_t deviceCount);

#endif
