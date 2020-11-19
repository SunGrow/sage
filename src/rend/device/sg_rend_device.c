#include "sg_rend_device.h"
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
