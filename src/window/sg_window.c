#include "sg_window.h"

void sgGetWindowSize(SgWindow* pWindow, int* xSize, int* ySize) {
	glfwGetWindowSize((GLFWwindow*)pWindow, xSize, ySize);
	return;
}

void sgSetCursorEnterCallback(SgWindow*              pWindow,
                              SgWindowCursorEnterFun callback) {
	glfwSetCursorEnterCallback((GLFWwindow*)pWindow,
	                           (GLFWcursorenterfun)callback);
	return;
}
