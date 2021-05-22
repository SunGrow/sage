#ifndef SAGE_JOBS_H_
#define SAGE_JOBS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_core.h"

// Job Pool: A pool for a Job Buffer to be allocated from
// Job DependencyLayer: An array of semaphores to be raised after the job
// execution. Once DependencyLayer is raised, a job buffer bind to a
// DependencyLayer could be executed Job Buffer: List of commands to be executed
// on a thread. Commands are not guaranteed to be executed in order or on a
// single thread. Has a DependencyLayer semaphore pointer to be flipped after
// execution and a DependencyLayer pointer that a job buffer depends on

SG_DEFINE_HANDLE(SgDependencyLayer);

typedef enum SgJobAllocFlagBits {
	SG_JOB_BUFFER_ALLOC_MAX_BIT,
} SgJobAllocFlagBits;
typedef SgFlags SgJobAllocFlags;

typedef struct SgJobBufferAllocInfo {
	SgJobAllocFlags flags;
} SgJobBufferAllocInfo;
SG_DEFINE_HANDLE(SgJobBuffer);
SgResult sgAllocateJobBuffers(SgJobBufferAllocInfo* pAllocInfo,
                              SgJobBuffer* pJobBuffer);

typedef void (*SgCommandFunc)(const void* input, void* object, void* output);
SG_DEFINE_HANDLE(SgCommand);
typedef struct SgCommandInfo {
	SgCommandFunc func;
	const void* input;
	void* object;
	void* output;
} SgCommandInfo;
SgResult sgAddCommand(SgJobBuffer jobBuffer, SgCommandInfo* pInfo);
/* */
SgResult sgSubmitCommand(SgCommandInfo* pInfo);

typedef enum SgJobSemaphoreCreateFlagBits {
	SG_JOB_SEMAPHORE_CREATE_MAX_BIT,
} SgJobSemaphoreCreateFlagBits;
typedef SgFlags SgJobSemaphoreCreateFlags;

typedef struct SgSemaphoreCreateInfo {
	SgJobSemaphoreCreateFlags flags;
} SgSemaphoreCreateInfo;

SG_DEFINE_HANDLE(SgSemaphore);
SgResult sgCreateSemaphore(SgSemaphoreCreateInfo* pCreateInfo,
                           SgSemaphore* pSemaphore);
void sgDestroySemaphore(SgSemaphore* pSemaphore);

typedef struct SgJobSubmitInfo {
	SgJobBuffer* pJobBuffers;
	SgSize jobBufferCount;
} SgJobSubmitInfo;

typedef struct SgJobQueueCreateInfo {
	SgSize threadCount;
} SgJobQueueCreateInfo;

SG_DEFINE_HANDLE(SgJobQueue);

SgResult sgJobQueueCreate(const SgJobQueueCreateInfo* pCreateInfo,
                          SgJobQueue* pJobQueue);
SgResult sgJobQueueSubmit(SgJobQueue* pJobQueue, SgJobSubmitInfo* pSubmitInfo);
SgResult sgJobQueueWait(SgJobQueue* pJobQueue);
SgResult sgJobQueueDelete(SgJobQueue* pJobQueue);

typedef enum SgJobPoolCreateFlagBits {
	SG_JOB_POOL_CREATE_MAX_BIT,
} SgJobPoolCreateFlagBits;
typedef SgFlags SgJobPoolCreateFlags;

typedef struct SgJobPoolCreateInfo {
	SgJobPoolCreateFlags flags;
	SgSize jobBufferCount;
} SgJobPoolCreateInfo;
SG_DEFINE_HANDLE(SgJobPool);

SgResult sgCreateJobPool(SgJobPoolCreateInfo* pCreateInfo, SgJobPool* pJobPool);

#ifdef __cplusplus
}
#endif

#endif
