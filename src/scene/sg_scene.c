#include "sg_scene.h"

void sgSceneInit(SgScene* pScene) {
	pScene->startTime = glfwGetTime();
	pScene->currentTime = glfwGetTime();
	pScene->previousTime = glfwGetTime();
}
void sgSceneUpdate(SgScene* pScene) {
	pScene->previousTime = pScene->currentTime;
	pScene->currentTime = glfwGetTime();
	pScene->deltaTime = pScene->currentTime - pScene->previousTime;
};

void sgInitTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform) {
	for (uint32_t i = 0; i < 4; ++i) {
		for (uint32_t j = 0; j < 4; ++j) {
			if (i == j)
				pTransformUniform->model[i][j] = 1;
			else
				pTransformUniform->model[i][j] = 0;
		}
	}

	v3 at;
	v3_add(at, pCamera->position, pCamera->front);
	lookat(pTransformUniform->view, pCamera->position, at, pCamera->up);
	perspective(pTransformUniform->projection, pCamera->fov, pCamera->aspectRatio, 0.1f, 10.0f);
}

void sgUpdateTransformUniform(const SgCamera* pCamera, SgTransformUniform *pTransformUniform) {
	v3 at;
	v3_add(at, pCamera->position, pCamera->front);
	lookat(pTransformUniform->view, pCamera->position, at, pCamera->up);
	perspective(pTransformUniform->projection, pCamera->fov, pCamera->aspectRatio, 0.1f, 10.0f);
}
