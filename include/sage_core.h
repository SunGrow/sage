#ifndef SAGE_CORE_H_
#define SAGE_CORE_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_platform.h"
#include "sage_types.h"

#define SG_DEFINE_HANDLE(object) typedef struct object##_T* object
#define BIT(n) (1UL << (n))

SG_DEFINE_HANDLE(SgFile);
SgResult sgOpenFile(const char* path, SgFile* pFile);
SgResult sgCloseFile(SgFile* pFile);

// Reserved for future use
#ifdef __cplusplus
}
#endif

#endif
