#ifndef SG_WINDOW_H_
#define SG_WINDOW_H_ 1

#include "GLFW/glfw3.h"
#include "sg_base.h"

typedef void (*SgWindowCursorEnterFun)(SgWindow, int);
void sgGetWindowSize(SgWindow* pWindow, int* xSize, int* ySize);
void sgSetCursorEnterCallback(SgWindow* pWindow,
                              SgWindowCursorEnterFun callback);

#endif
