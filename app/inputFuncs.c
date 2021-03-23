#include "inputFuncs.h"

void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo, SgCamera* pCamera) {
	/* Move */
	v3 tmp;
	v3 right;
	v3_cross(right, pCamera->front, pCamera->up);
	v3_normalize_to(right, right);

	v3_scale_by(tmp, pCamera->speed * pTransformInfo->deltaTime * pTransformInfo->moveDirection[0], pCamera->front);
	v3_add(pCamera->position, tmp, pCamera->position);
	v3_scale_by(tmp, pCamera->speed * pTransformInfo->deltaTime * pTransformInfo->moveDirection[1], pCamera->up);
	v3_add(pCamera->position, tmp, pCamera->position);
	v3_scale_by(tmp, pCamera->speed * pTransformInfo->deltaTime * pTransformInfo->moveDirection[2], right);
	v3_add(pCamera->position, tmp, pCamera->position);

	/* Rotate */
	v2 offset;
	v2_scale_by(offset, pTransformInfo->deltaTime, pTransformInfo->cursorOffset);
	v2_scale_by(offset, pCamera->sensitivity, offset);

	static float yaw, pitch;

	yaw += offset[0];
	pitch += offset[1];

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	v3 direction;
	direction[0] = cosf(deg_to_rad(yaw)) * cosf(deg_to_rad(pitch));
	direction[1] = sinf(deg_to_rad(pitch));
	direction[2] = sinf(deg_to_rad(yaw)) * cosf(deg_to_rad(pitch));
	v3_normalize_to(pCamera->front, direction);

	return;
}

void keyPressForward(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[0] = isPressed ? 1.0f : 0.0f;
	return;
}
void keyPressUp(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[1] = isPressed ? 1.0f : 0.0f;
	return;
}
void keyPressLeft(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[2] = isPressed ? -1.0f : 0.0f;
	return;
}
void keyPressRight(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[2] = isPressed ? 1.0f : 0.0f;
	return;
}
void keyPressDown(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[1] = isPressed ? -1.0f : 0.0f;
	return;
}
void keyPressBack(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[0] = isPressed ? -1.0f : 0.0f;
	return;
}

void keyShoot(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	if (isPressed)
		log_warn("shooty shooty bang bang");
	return;
}

void rotateCamera(SgBool isPressed, double rangeX, double rangeY, SgActor actor, SgWindow pWindow) {
	int wsizex, wsizey;
	SgCameraTransformInfo* pTransformInfo = (SgCameraTransformInfo*) actor;
	sgGetWindowSize(pWindow, &wsizex, &wsizey);
	if (rangeX == wsizex/2 && rangeY == wsizey/2) {
		pTransformInfo->camera.cursorPosition[0] = rangeX;
		pTransformInfo->camera.cursorPosition[1] = rangeY;
		return;
	}
	pTransformInfo->cursorOffset[0] = rangeX - pTransformInfo->camera.cursorPosition[0];
	pTransformInfo->cursorOffset[1] = pTransformInfo->camera.cursorPosition[1] - rangeY;

	pTransformInfo->camera.cursorPosition[0] = rangeX;
	pTransformInfo->camera.cursorPosition[1] = rangeY;
}
