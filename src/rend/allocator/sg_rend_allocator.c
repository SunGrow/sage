#include "sg_rend_allocator.h"
#include "vk_mem_alloc.h"


SgResult createAllocator(const VkPhysicalDevice* pPhysicalDevice, const VkDevice* pDevice, const VkInstance* pInstance, VmaAllocator* pAllocator) {
	VmaAllocatorCreateInfo allocatorInfo = {
	    .physicalDevice = *pPhysicalDevice,
	    .device = *pDevice,
	    .instance = *pInstance,
	    .vulkanApiVersion = VK_API_VERSION_1_2,
	};

	if(vmaCreateAllocator(&allocatorInfo, pAllocator)) {
		sgLogError("[AppInit]: Allocator Creation failure");
	} else {
		sgLogInfo_Debug("[AppInit]: Allocator Creation successfull");
	}

	return SG_SUCCESS;
}
