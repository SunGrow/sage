#include "sg_rend_surface.h"
#include "log.h"
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

SgResult createWindowSurface(SgApp *pApp) {
	glfwCreateWindowSurface(pApp->instance, pApp->pWindow, VK_NULL_HANDLE, &pApp->surface);
	if (pApp->surface) {
		sgLogInfo_Debug("[AppInit]: Vulkan Surface created");
	} else {
		sgLogFatal("[AppInit]: Vulkan Surface creation failure");
		return -1;
	}

	return SG_SUCCESS;
}

VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	uint32_t formatCount = 0;
	VkSurfaceFormatKHR format;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, VK_NULL_HANDLE);
	VkSurfaceFormatKHR *pFormats = malloc(formatCount * sizeof(*pFormats));
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, pFormats);
	if (formatCount == 1 && pFormats[0].format == VK_FORMAT_UNDEFINED) {
		free(pFormats);
		sgLogInfo("[AppInit]: Surface Format not specified");
		return (VkSurfaceFormatKHR){
		    .format = VK_FORMAT_B8G8R8A8_UNORM,
		    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		};
	}
	for (uint32_t i = 0; i < formatCount; ++i) {
		if (pFormats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
		    pFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = pFormats[i];
			free(pFormats);
			sgLogInfo_Debug("[AppInit]: Surface Format found");
			return format;
		}
	}
	format = pFormats[0];
	free(pFormats);
	sgLogWarn("[AppInit]: Surface Format not found");
	return format;
}

VkPresentModeKHR getSurfacePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	uint32_t modeCount = 0;
	VkPresentModeKHR result;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, VK_NULL_HANDLE);
	VkPresentModeKHR *pPresentModes = malloc(modeCount * sizeof(*pPresentModes)); 
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modeCount, pPresentModes);
	for (uint32_t i = 0; i < modeCount; ++i) {
		if (pPresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			result = pPresentModes[i];
			free(pPresentModes);
			sgLogInfo_Debug("[AppInit]: Surface Present Mode found");
			return result;
		}
	}
	result = pPresentModes[0];
	free(pPresentModes);
	sgLogWarn("[AppInit]: Surface Present Mode not found");
	return result;
}

SgResult getSurfaceAttributes(SgApp *pApp) {
	pApp->surfaceAttributes.format = getSurfaceFormat(pApp->physicalDevice, pApp->surface);
	pApp->surfaceAttributes.presentMode = getSurfacePresentMode(pApp->physicalDevice, pApp->surface);
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pApp->physicalDevice, pApp->surface, &pApp->surfaceAttributes.surfaceCapabilities) == VK_SUCCESS) {
		sgLogInfo_Debug("[AppInit]: Surface Capabilities acquired successfully");
	} else {
		sgLogWarn("[AppInit]: Surface Capabilities request error");
	}
	sgLogInfo_Debug("[AppInit]: Surface Attributes are initialized");
	return SG_SUCCESS;
}
