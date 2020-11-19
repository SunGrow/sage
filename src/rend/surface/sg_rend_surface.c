#include "sg_rend_surface.h"
#include "log.h"
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

SgResult createWindowSurface(SgApp *pApp) {
	glfwCreateWindowSurface(pApp->instance, pApp->pWindow, VK_NULL_HANDLE, &pApp->surface);
	if (pApp->surface) {
		log_info("[AppInit]: Vulkan Surface created");
	} else {
		log_fatal("[AppInit]: Vulkan Surface creation failure");
		return -1;
	}

	return SG_SUCCESS;
}

VkSurfaceFormatKHR getSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	uint32_t formatCount = 0;
	VkSurfaceFormatKHR format;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, VK_NULL_HANDLE);
	VkSurfaceFormatKHR *pformats = calloc(formatCount, sizeof(VkSurfaceFormatKHR));
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, pformats);
	if (formatCount == 1 && pformats[0].format == VK_FORMAT_UNDEFINED) {
		free(pformats);
		log_info("[AppInit]: Surface Format not specified");
		return (VkSurfaceFormatKHR){
		    .format = VK_FORMAT_B8G8R8A8_UNORM,
		    .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		};
	}
	for (uint32_t i = 0; i < formatCount; ++i) {
		if (pformats[i].format == VK_FORMAT_B8G8R8A8_UNORM &&
		    pformats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			format = pformats[i];
			free(pformats);
			log_info("[AppInit]: Surface Format found");
			return format;
		}
	}
	format = pformats[0];
	free(pformats);
	log_warn("[AppInit]: Surface Format not found");
	return format;
}

VkPresentModeKHR getSurfacePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	uint32_t modecnt = 0;
	VkPresentModeKHR result;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modecnt, VK_NULL_HANDLE);
	VkPresentModeKHR *ppresmodes = calloc(modecnt, sizeof(VkPresentModeKHR)); 
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &modecnt, ppresmodes);
	for (uint32_t i = 0; i < modecnt; ++i) {
		if (ppresmodes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			result = ppresmodes[i];
			free(ppresmodes);
			log_info("[AppInit]: Surface Present Mode found");
			return result;
		}
	}
	result = ppresmodes[0];
	free(ppresmodes);
	log_warn("[AppInit]: Surface Present Mode not found");
	return result;
}

SgResult getSurfaceAttributes(SgApp *pApp) {
	pApp->surfaceAttributes.format = getSurfaceFormat(pApp->physicalDevice, pApp->surface);
	pApp->surfaceAttributes.presentMode = getSurfacePresentMode(pApp->physicalDevice, pApp->surface);
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pApp->physicalDevice, pApp->surface, &pApp->surfaceAttributes.surfaceCapabilities) == VK_SUCCESS) {
		log_info("[AppInit]: Surface Capabilities acquired successfully");
	} else {
		log_warn("[AppInit]: Surface Capabilities request error");
	}
	log_info("[AppInit]: Surface Attributes are initialized");
	return SG_SUCCESS;
}
