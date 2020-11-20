#include "sg_rend_allocator.h"
#include "vk_mem_alloc.h"
#include "log.h"


SgResult createAllocator(SgApp *pApp) {
	VmaAllocatorCreateInfo allocatorInfo = {
	    .physicalDevice = pApp->physicalDevice,
	    .device = pApp->device,
	    .instance = pApp->instance,
	    .vulkanApiVersion = VK_API_VERSION_1_2,
	};

	if(vmaCreateAllocator(&allocatorInfo, &pApp->allocator)) {
		log_error("[AppInit]: Allocator Creation failure");
	} else {
		log_info("[AppInit]: Allocator Creation successfull");
	}

	return SG_SUCCESS;
}
