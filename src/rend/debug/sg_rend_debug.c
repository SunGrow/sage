#include "sage_base.h"
#include "sg_rend.h"
#include "log.h"

#ifdef _DEBUG
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT flags,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pData,
    void* pUserData){
	(flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	                       ? sgLogError("[%s]: %s", pData->pMessageIdName, pData->pMessage)
	                       : (flags & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	                             ? sgLogWarn("[%s]: %s", pData->pMessageIdName, pData->pMessage)
	                             : sgLogInfo("[%s]: %s", pData->pMessageIdName, pData->pMessage);
	return VK_FALSE;
}

SgResult registerDebugCallback(SgApp *pApp) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {
	    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
	    .pfnUserCallback = debugCallback,
	};

	VkResult res = vkCreateDebugUtilsMessengerEXT(pApp->instance, &createInfo, 0, &pApp->debugCallback);

	if (!res) {
		sgLogInfo_Debug("[AppInit]: Vulkan debug report callback is setup");
	} else {
		sgLogWarn("[AppInit]: Vulkan debug report callback setup failure");
	}

	return SG_SUCCESS;
}
#endif
