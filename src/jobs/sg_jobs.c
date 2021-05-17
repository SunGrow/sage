#include "sg_jobs.h"

SgResult sgCreateSemaphore(SgSemaphoreCreateInfo* pCreateInfo,
                           SgSemaphore** ppSemaphore) {
	SgSemaphore* pSemaphore = *ppSemaphore;
	SG_CALLOC_NUM(pSemaphore, 1);
	pSemaphore->pName = pCreateInfo->pName;
	*ppSemaphore = pSemaphore;
	return SG_SUCCESS;
}

void sgDestroySemaphore(SgSemaphore** ppSemaphore) {
	if (*ppSemaphore) {
		free(*ppSemaphore);
	}
	return;
}

void* signalingCommandFunc(void* pData) {
	SgSignalingCommand* pSignalingCommand = pData;
	const void* pInput = pSignalingCommand->pCommand->input;
	void* pObject = pSignalingCommand->pCommand->object;
	void* pOutput = pSignalingCommand->pCommand->output;
	pSignalingCommand->pCommand->func(pInput, pObject, pOutput);
	pSignalingCommand->pSemaphore->semaphore = 1;
	return NULL;
}

SgResult sgExecuteJobBuffer(SgJobBuffer* pJobBuffer, pthread_t* pThread) {
	SgBool isReady = 1;
	for (uint32_t i = 0; i < pJobBuffer->pDependencyLayer->semaphoreCount; ++i) {
		isReady &= (pJobBuffer->pDependencyLayer->ppSemaphores[i])->semaphore;
	}
	if (isReady) {
		for (uint32_t i = 0; i < pJobBuffer->commandCount; ++i) {
			SgSignalingCommand signaligCommand = {
			    .pCommand = &pJobBuffer->pCommands[i],
			    .pSemaphore = &pJobBuffer->pCommandSemaphores[i],
			};
			pthread_create(pThread, NULL, signalingCommandFunc, &signaligCommand);
			pthread_detach(*pThread);
		}
	}
	return SG_SUCCESS;
}

SgResult sgJobBufferUpdateSemaphore(SgJobBuffer* pJobBuffer) {
	SgBool isReady = 1;
	for (uint32_t i = 0; i < pJobBuffer->commandCount; ++i) {
		isReady &= pJobBuffer->pCommandSemaphores[i].semaphore;
	}
	if (isReady) {
		pJobBuffer->pSemaphore->semaphore = 1;
	}
	return SG_SUCCESS;
}

SgResult sgJobQueueSubmit(const SgJobQueue* pJobQueue,
                          const SgJobQueueSubmitInfo* pSubmitInfo) {
	for (uint32_t i = 0; i < pSubmitInfo->jobBufferCount; ++i) {
		sgJobBufferUpdateSemaphore(&pSubmitInfo->pJobBuffers[i]);
	}
	for (uint32_t i = 0; i < pSubmitInfo->jobBufferCount; ++i) {
		sgExecuteJobBuffer(&pSubmitInfo->pJobBuffers[i],
		                   &pJobQueue->pThreads[i % pJobQueue->threadCount]);
	}
	return SG_SUCCESS;
}

SgResult sgJobQueueInit(SgJobQueueInitInfo* pInitInfo,
                        SgJobQueue** ppJobQueue) {
	SgJobQueue* pJobQueue = *ppJobQueue;
	SG_CALLOC_NUM(pJobQueue, 1);
	pJobQueue->threadCount = pInitInfo->threadCount;
	SG_CALLOC_NUM(pJobQueue->pThreads, pInitInfo->threadCount);
	return SG_SUCCESS;
}
