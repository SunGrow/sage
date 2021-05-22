#ifndef SG_JOBS
#define SG_JOBS 1
#include <pthread.h>
#include <stdatomic.h>
#include "sg_base.h"

typedef void (*SgCommandFunc)(const void* input, void* object, void* output);
typedef struct SgCommand {
	SgCommandFunc func;
	const void* input;
	void* object;
	void* output;
} SgCommand;

typedef enum SgJobSemaphoreCreateFlagBits {
	SG_JOB_SEMAPHORE_CREATE_MAX_BIT,
} SgJobSemaphoreCreateFlagBits;
typedef SgFlags SgJobSemaphoreCreateFlags;
typedef struct SgSemaphoreCreateInfo {
	const char* pName;
	SgJobSemaphoreCreateFlags flags;
} SgSemaphoreCreateInfo;

typedef struct SgSemaphore {
	atomic_uint_fast8_t semaphore;
	const char* pName;
} SgSemaphore;
SgResult sgCreateSemaphore(SgSemaphoreCreateInfo* pCreateInfo,
                           SgSemaphore** ppSemaphore);
void sgDestroySemaphore(SgSemaphore** ppSemaphore);

typedef struct SgDependencyLayer {
	SgSemaphore** ppSemaphores;
	SgSize semaphoreCount;
} SgDependencyLayer;

typedef struct SgJobBuffer {
	SgCommand* pCommands;
	SgSemaphore* pCommandSemaphores;
	SgSize commandCount;
	SgDependencyLayer* pDependencyLayer;
	SgSize dependencyLayerSemaphoreCount;
	SgSemaphore* pSemaphore;
} SgJobBuffer;

typedef enum SgJobPoolCreateFlagBits {
	SG_JOB_POOL_CREATE_MAX_BIT,
} SgJobPoolCreateFlagBits;
typedef SgFlags SgJobPoolCreateFlags;

typedef struct SgJobPoolCreateInfo {
	SgJobPoolCreateFlags flags;
	SgSize jobBufferCount;
} SgJobPoolCreateInfo;

typedef struct SgJobPool {
	SgJobBuffer* pBuffers;
	SgSize bufferCount;
} SgJobPool;

SgResult sgCreateJobPool(SgJobPoolCreateInfo* pCreateInfo,
                         SgJobPool** ppJobPool);

typedef enum SgJobAllocFlagBits {
	SG_JOB_BUFFER_ALLOC_MAX_BIT,
} SgJobAllocFlagBits;
typedef SgFlags SgJobAllocFlags;

typedef struct SgJobBufferAllocInfo {
	SgJobPool* pJobPool;
	SgJobAllocFlags allocFlags;
	SgSize commandCount;
} SgJobBufferAllocInfo;
SgResult sgAllocateJobBuffers(SgJobBufferAllocInfo* pAllocInfo,
                              SgJobBuffer* pJobBuffer);

typedef struct SgSignalingCommand {
	SgCommand* pCommand;
	SgSemaphore* pSemaphore;
} SgSignalingCommand;

SgResult sgExecuteJobBuffer(SgJobBuffer* pJobBuffer, pthread_t* pThread);
SgResult sgJobBufferUpdateSemaphore(SgJobBuffer* pJobBuffer);

typedef struct SgJobQueueInitInfo {
	SgSize threadCount;
} SgJobQueueInitInfo;

typedef struct SgJobQueue {
	pthread_t* pThreads;
	SgSize threadCount;
} SgJobQueue;

SgResult sgJobQueueInit(SgJobQueueInitInfo* pInitInfo, SgJobQueue** ppJobQueue);

typedef struct SgJobQueueSubmitInfo {
	SgJobBuffer* pJobBuffers;
	SgSize jobBufferCount;
} SgJobQueueSubmitInfo;

SgResult sgJobQueueSubmit(const SgJobQueue* pJobQueue,
                          const SgJobQueueSubmitInfo* pSubmitInfo);

#endif
