#include "sg_rend_device.h"

#include <stdlib.h>

// could replace with trying to get all of the families
uint32_t getGraphicsFamilyIndex(VkPhysicalDevice pd) {
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, 0);
	VkQueueFamilyProperties* queues;
	SG_MALLOC_NUM(queues, queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, queues);

	// Too lazy to handle multiple families case. Might do it later
	for (uint32_t i = 0; i < queueCount; ++i)
		if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		    && (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
			free(queues);
			return i;
		}

	free(queues);
	return VK_QUEUE_FAMILY_IGNORED;
}

VkPhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface,
                                    VkPhysicalDevice* pPhysicalDevices,
                                    uint32_t deviceCount) {
	VkBool32 boolres;
	VkPhysicalDevice preferedDevice = 0;
	VkPhysicalDevice fallbackDevice = 0;
	for (uint32_t i = 0; i < deviceCount; ++i) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(pPhysicalDevices[i], &props);
		sgLogInfo_Debug("[AppInit]: GPU %s found", props.deviceName);
		uint32_t familyIdx = getGraphicsFamilyIndex(pPhysicalDevices[i]);
		if (familyIdx == VK_QUEUE_FAMILY_IGNORED) {
			continue;
		}

		if (props.apiVersion < VK_API_VERSION_1_2) {
			continue;
		}
		vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevices[i], familyIdx,
		                                     surface, &boolres);
		if (boolres != VK_TRUE) {
			continue;
		}

		if (!preferedDevice
		    && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			preferedDevice = pPhysicalDevices[i];
		}

		if (!fallbackDevice) {
			fallbackDevice = pPhysicalDevices[i];
		}
	}
	VkPhysicalDevice result = preferedDevice ? preferedDevice : fallbackDevice;

	if (result) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(result, &properties);
		sgLogInfo_Debug("[AppInit]: Selected GPU %s", properties.deviceName);
	} else {
		sgLogError("[AppInit]: No suitable GPUs found");
	}
	return result;
}

SgResult getPhysicalDevice(SgPhysicalDeviceGetInfo* pGetInfo,
                           VkPhysicalDevice* pPhysicalDevice) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(*pGetInfo->pInstance, &deviceCount,
	                           VK_NULL_HANDLE);
	VkPhysicalDevice* pPhysicalDevices =
	    malloc(deviceCount * sizeof(*pPhysicalDevices));
	vkEnumeratePhysicalDevices(*pGetInfo->pInstance, &deviceCount,
	                           pPhysicalDevices);
	*pPhysicalDevice =
	    pickPhysicalDevice(*pGetInfo->pSurface, pPhysicalDevices, deviceCount);
	if (*pPhysicalDevice) {
		sgLogInfo_Debug("[AppInit]: Vulkan Physical Device found");
	} else {
		sgLogFatal("[AppInit]: Vulkan Physical Device not found");
	}
	free(pPhysicalDevices);
	return SG_SUCCESS;
}

static const char* deviceExtensionNames[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

SgResult getLogicalDevice(SgLogicalDeviceGetInfo* pGetInfo, VkDevice* pDevice) {
	VkDeviceQueueCreateInfo* pQueueCreateInfos;
	SG_CALLOC_NUM(pQueueCreateInfos, pGetInfo->createInfosCount);
	for (uint32_t i = 0; i < pGetInfo->createInfosCount; ++i) {
		pQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		pQueueCreateInfos[i].queueFamilyIndex =
		    pGetInfo->pQueueCreateInfos[i].queueIndex;
		pQueueCreateInfos[i].pQueuePriorities =
		    pGetInfo->pQueueCreateInfos[i].pQueuePriorities;
		pQueueCreateInfos[i].queueCount = pGetInfo->pQueueCreateInfos[i].queueCount;
	}
	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .queueCreateInfoCount = pGetInfo->createInfosCount,
	    .pQueueCreateInfos = pQueueCreateInfos,
	    .enabledExtensionCount = NUMOF(deviceExtensionNames),
	    .ppEnabledExtensionNames = deviceExtensionNames,
	};
	vkCreateDevice(pGetInfo->physicalDevice, &createInfo, VK_NULL_HANDLE,
	               pDevice);
	if (*pDevice) {
		sgLogInfo_Debug("[AppInit]: Logical Device created successfully");
	} else {
		sgLogWarn("[AppInit]: Logical Device creation failure");
		return -1;
	}
	free(pQueueCreateInfos);
	return SG_SUCCESS;
}
