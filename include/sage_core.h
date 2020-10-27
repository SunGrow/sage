#ifndef SAGE_CORE_H_
#define SAGE_CORE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_platform.h"

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;

typedef struct SgAppInfo {
	const char*            pAppName;
} SgAppInfo;

// Reserved for future use
typedef enum SgAppCreateFlags {
} SgAppCreateFlags;

typedef struct SgAppCreateInfo {
	const SgAppInfo*       pAppInfo;
	SgAppCreateFlags       flags;
} SgAppCreateInfo;

/* SgResult */void sgCreateApp();

#ifdef __cplusplus
}
#endif

#endif
