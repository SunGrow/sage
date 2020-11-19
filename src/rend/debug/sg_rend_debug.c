#include "sage_base.h"
#include "sg_rend.h"
#include "log.h"

#ifdef _DEBUG
static VkBool32 VKAPI_CALL
debugReportCallback(VkDebugReportFlagsEXT flags,
                    VkDebugReportObjectTypeEXT objectType, uint64_t object,
                    size_t location, int32_t messageCode,
                    const char *pLayerPrefix, const char *pMessage,
                    void *pUserData) {
	(flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	                       ? log_error("%s", pMessage)
	                       : (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	                             ? log_warn("%s", pMessage)
	                             : log_info("%s", pMessage);
	return VK_FALSE;
}

VkDebugReportCallbackEXT registerDebugCallback(SgApp *pApp) {
	VkDebugReportCallbackCreateInfoEXT createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT,
	    .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
	             VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
	             VK_DEBUG_REPORT_ERROR_BIT_EXT,
	    .pfnCallback = debugReportCallback,
	};

	VkResult res = vkCreateDebugReportCallbackEXT(pApp->instance, &createInfo, 0, &pApp->debugCallback);

	if (!res) {
		log_info("[AppInit]: Vulkan debug report callback is setup");
	} else {
		log_warn("[AppInit]: Vulkan debug report callback setup failure");
	}

	return SG_SUCCESS;
}
#endif
