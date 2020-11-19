#include "sg_rend_device.h"
#include "sage_base.h"
#include <stdlib.h>
#include "log.h"

// could replace with trying to get all of the families
uint32_t getGraphicsFamilyIndex(VkPhysicalDevice pd) {
	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, 0);

	VkQueueFamilyProperties queues[queueCount]; // using vla
	vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueCount, queues);

	// Too lazy to handle multiple families case. Might do it later
	for (uint32_t i = 0; i < queueCount; ++i)
		if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		    (queues[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
			return i;
		}

	return VK_QUEUE_FAMILY_IGNORED;
}

VkPhysicalDevice pickPhysicalDevice(VkSurfaceKHR surface, VkPhysicalDevice *pPhysicalDevices, uint32_t deviceCount) {
	VkBool32 boolres;
	VkPhysicalDevice preferedDevice = 0;
	VkPhysicalDevice fallbackDevice = 0;
	for (uint32_t i = 0; i < deviceCount; ++i) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(pPhysicalDevices[i], &props);
		log_info("[AppInit]: GPU %s found", props.deviceName);
		uint32_t familyIdx = getGraphicsFamilyIndex(pPhysicalDevices[i]);
		if (familyIdx == VK_QUEUE_FAMILY_IGNORED) {
			continue;
		}

		if (props.apiVersion < VK_API_VERSION_1_2) {
			continue;
		}
		vkGetPhysicalDeviceSurfaceSupportKHR(pPhysicalDevices[i], familyIdx, surface, &boolres);
		if (boolres != VK_TRUE) {
			continue;
		}

		if (!preferedDevice && props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
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
		log_info("[AppInit]: Selected GPU %s", properties.deviceName);
	} else {
		log_error("[AppInit]: No suitable GPUs found");
	}
	return result;
}

SgResult getPhysicalDevice(SgApp *pApp) {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, VK_NULL_HANDLE);
	VkPhysicalDevice *pPhysicalDevices = calloc(deviceCount, sizeof(VkPhysicalDevice));
	vkEnumeratePhysicalDevices(pApp->instance, &deviceCount, pPhysicalDevices);
	pApp->physicalDevice = pickPhysicalDevice(pApp->surface, pPhysicalDevices, deviceCount);
	pApp->graphicsQueueFamilyIdx = getGraphicsFamilyIndex(pApp->physicalDevice);
	if (pApp->physicalDevice) {
		log_info("[AppInit]: Vulkan Physical Device found");
	} else {
		log_fatal("[AppInit]: Vulkan Physical Device not found");
	}
	free(pPhysicalDevices);
	return SG_SUCCESS;
}

static const char *deviceExtensionNames[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

SgResult getLogicalDevice(SgApp *pApp) {
	// Should really come from a pApp
	float pQueuePriorities[] = {1.0f};
	// Should be abstracted away
	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	    .queueFamilyIndex = pApp->graphicsQueueFamilyIdx,
	    .queueCount = 1,
	    .pQueuePriorities = pQueuePriorities,
	};
	VkDeviceQueueCreateInfo pQueueCreateInfos[] = {
	    graphicsQueueCreateInfo,
	};
	VkDeviceCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	    .queueCreateInfoCount = NUMOF(pQueueCreateInfos),
	    .pQueueCreateInfos = pQueueCreateInfos,
	    .enabledExtensionCount = NUMOF(deviceExtensionNames),
	    .ppEnabledExtensionNames = deviceExtensionNames,
	};
	vkCreateDevice(pApp->physicalDevice, &createInfo, VK_NULL_HANDLE, &pApp->device);
	if (pApp->device) {
		log_info("[AppInit]: Logical Device created successfully");
	} else {
		log_warn("[AppInit]: Logical Device creation failure");
		return -1;
	}
	return SG_SUCCESS;
}
