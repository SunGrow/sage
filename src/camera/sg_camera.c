#include "sg_camera.h"

void sgTimedCameraInit(SgTimedCamera* pTimedCamera) {
	pTimedCamera->startTime    = glfwGetTime();
	pTimedCamera->currentTime  = glfwGetTime();
	pTimedCamera->previousTime = glfwGetTime();
}
void sgTimedCameraUpdate(SgTimedCamera* pTimedCamera) {
	pTimedCamera->previousTime = pTimedCamera->currentTime;
	pTimedCamera->currentTime  = glfwGetTime();
	pTimedCamera->deltaTime =
	    pTimedCamera->currentTime - pTimedCamera->previousTime;
}

void sgInitTransformUniform(const SgTmpCamera* pCamera,
                            SgTransformUniform* pTransformUniform) {
	for (uint32_t i = 0; i < 4; ++i) {
		for (uint32_t j = 0; j < 4; ++j) {
			if (i == j)
				pTransformUniform->model[i][j] = 1;
			else
				pTransformUniform->model[i][j] = 0;
		}
	}
}

void sgUpdateTransformUniform(const SgTmpCamera* pCamera,
                              SgTransformUniform* pTransformUniform) {
	v3 at;
	v3_add(at, pCamera->position, pCamera->front);
	lookat(pTransformUniform->view, pCamera->position, at, pCamera->up);
	perspective(pTransformUniform->projection, pCamera->fov, pCamera->aspectRatio,
	            0.01f, 60.0f);
}
