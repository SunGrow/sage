#ifndef SAGE_WINDOW_H_
#define SAGE_WINDOW_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include "sage_core.h"

SG_DEFINE_HANDLE(SgWindow);
typedef void (*SgWindowCursorEnterFun)(SgWindow, int);
void sgGetWindowSize(SgWindow window, int* xSize, int* ySize);
void sgSetCursorEnterCallback(SgWindow window, SgWindowCursorEnterFun callback);

#ifdef __cplusplus
}
#endif

#endif
