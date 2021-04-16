#ifndef SAGE_JOBS_H_
#define SAGE_JOBS_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_core.h"

// Job Pool: A pool for a Job Buffer to be allocated from
// Job DependencyLayer: An array of semaphores to be raised after the job execution. Once DependencyLayer is raised, a job buffer bind to a DependencyLayer could be executed
// Job Buffer: List of commands to be executed on a thread. Commands are not guaranteed to be executed in order or on a single thread. Has a DependencyLayer semaphore pointer to be flipped after execution and a DependencyLayer pointer that a job buffer depends on 

SG_DEFINE_HANDLE(SgDependencyLayer);

typedef enum SgJobPoolCreateFlagBits {
	SG_JOB_POO_CREATEL_MAX_BIT,
} SgJobPoolCreateFlagBits;
typedef SgFlags SgJobPoolCreateFlags;

typedef struct SgJobPoolCreateInfo {
} SgJobPoolCreateInfo;
SG_DEFINE_HANDLE(SgJobPool);

SgResult sgCreateJobPool(SgJobPoolCreateInfo* pCreateInfo, SgJobPool* pJobPool);

typedef enum SgJobAllocFlagBits {
	SG_JOB_BUFFER_ALLOC_MAX_BIT,
} SgJobAllocFlagBits;
typedef SgFlags SgJobAllocFlags;

typedef struct SgJobBufferAllocInfo {
} SgJobBufferAllocInfo;
SG_DEFINE_HANDLE(SgJobBuffer);
SgResult sgAllocateJobBuffers(SgJobBufferAllocInfo* pAllocInfo, SgJobBuffer* pJobBuffer);

/*
 * typedef struct SgJobBuffer {
 *     SgCommand*         pCommands;
 *     uint32_t           commandCount;
 *     SgDependencyLayer* pDependencyLayer;
 *     uint32_t           dependencyLayerSemaphoreCount;
 *     SgSemaphore*       pSemaphore;
 * } SgJobBuffer;
 */

/*
 * typedef struct SgCommand {
 *     SgCommandFunc func;
 *     const void* input;
 *     void* object;
 *     void* output;
 * } SgCommand;
 */


typedef void (*SgCommandFunc) (const void* input, void* object, void* output);
SG_DEFINE_HANDLE(SgCommand);
typedef struct SgCommandAddInfo {
	SgCommandFunc func;
	const void* input;
	void* object;
	void* output;
} SgCommandAddInfo;
SgResult sgAddCommand(SgJobBuffer jobBuffer, SgCommandAddInfo* addInfo);

typedef enum SgJobSemaphoreCreateFlagBits {
	SG_JOB_SEMAPHORE_CREATE_MAX_BIT,
} SgJobSemaphoreCreateFlagBits;
typedef SgFlags SgJobSemaphoreCreateFlags;

typedef struct SgJobSemaphoreCreateInfo {
} SgJobSemaphoreCreateInfo;
SG_DEFINE_HANDLE(SgSemaphore);

typedef struct SgJobSubmitInfo {
	SgJobBuffer* pJobBuffers;
	uint32_t     jobBufferCount;
} SgJobSubmitInfo;

#ifdef __cplusplus
}
#endif

#endif
