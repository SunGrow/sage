#ifndef SAGE_CORE_H_
#define SAGE_CORE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_platform.h"

#define SG_DEFINE_HANDLE( object ) typedef struct object##_T *object;


typedef uint32_t SgFlags;

typedef struct SgAppInfo {
	const char*            pAppName;
} SgAppInfo;

// Reserved for future use
typedef SgFlags SgAppCreateFlags;

typedef struct SgAppCreateInfo {
	const SgAppInfo*       pAppInfo;
	SgAppCreateFlags       flags;
} SgAppCreateInfo;

/* SgResult */void sgCreateApp();

#ifdef __cplusplus
}
#endif

#endif
