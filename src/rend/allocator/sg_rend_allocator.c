#include "sg_rend_allocator.h"
#include "vk_mem_alloc.h"


SgResult createAllocator(SgApp *pApp) {
	VmaAllocatorCreateInfo allocatorInfo = {
	    .physicalDevice = pApp->physicalDevice,
	    .device = pApp->device,
	    .instance = pApp->instance,
	    .vulkanApiVersion = VK_API_VERSION_1_2,
	};

	if(vmaCreateAllocator(&allocatorInfo, &pApp->allocator)) {
		sgLogError("[AppInit]: Allocator Creation failure");
	} else {
		sgLogInfo_Debug("[AppInit]: Allocator Creation successfull");
	}

	return SG_SUCCESS;
}
