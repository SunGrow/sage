#include "sg_rend_command.h"
#include "log.h"

typedef struct ThreadCommandPoolCreateInfo {
	SgApp *pApp;
	uint32_t threadIndex;
} ThreadCommandPoolCreateInfo;

static void* createCommandPool(void* threadCommandPoolInfo) {
	VkResult vulkRes;
	VkCommandPool commandpool = 0;
	ThreadCommandPoolCreateInfo* info = (ThreadCommandPoolCreateInfo *) threadCommandPoolInfo;
	SgApp* pTmpApp = info->pApp;
	uint32_t index = info->threadIndex;

	VkCommandPoolCreateInfo createInfo = {
	    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
	    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
	    .queueFamilyIndex = pTmpApp->graphicsQueueFamilyIdx,
	};

	for (uint32_t i = 0; i < SG_FRAME_QUEUE_LENGTH; ++i) {
		vulkRes = vkCreateCommandPool(pTmpApp->device, &createInfo, VK_NULL_HANDLE, &commandpool);
		if (vulkRes) {
			log_warn("[AppInit]: Command Pool creation failure %d", index);
			return (void*) -1;
		}
		pTmpApp->pCommandPools[index*SG_FRAME_QUEUE_LENGTH + i] = commandpool;
	}
	return (void*) SG_SUCCESS;
}

SgResult initCommandPools(SgApp *pApp) {
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// Submit pool create commands
	for (uint32_t i = 0; i < SG_THREADS_COUNT; ++i) {
		ThreadCommandPoolCreateInfo info = {
			.pApp = pApp,
			.threadIndex = i,
		};
		pthread_create(&pApp->threads[i], &attr, createCommandPool, &info);
		int rc = pthread_join(pApp->threads[i], NULL);
		if(rc) {
              log_error("[AppInit]: Return code from pthread_join() is %d\n", rc);
        }
	}
	for (uint32_t i = 0; i < SG_THREADS_COUNT; ++i) {
		for (uint32_t j = 0; j < SG_FRAME_QUEUE_LENGTH; ++j) {
			if(pApp->pCommandPools[i*SG_FRAME_QUEUE_LENGTH + j] == VK_NULL_HANDLE) {
				log_warn("[AppInit]: CommandPool %d not initialized", i*SG_FRAME_QUEUE_LENGTH + j);
			}
		}
	}
	log_info("[AppInit]: Command Pools initialization completed");
	return SG_SUCCESS;
}
