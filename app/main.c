#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_res.h"
#include "sage_scene.h"
#include "log.h"
#include "math.h"

enum KeyBindings {
	FOWARD_KEY = GLFW_KEY_I,
	BACK_KEY = GLFW_KEY_N,
	LEFT_KEY = GLFW_KEY_H,
	RIGHT_KEY = GLFW_KEY_L,
	UP_KEY = GLFW_KEY_K,
	DOWN_KEY = GLFW_KEY_J,
};

typedef struct SgCameraTransformInfo {
	v3              moveDirection;
	v2              cursorOffset;
	double          deltaTime;
} SgCameraTransformInfo;

SgScene scene = {0};

SgCamera camera = {
    .position = {-2.0f, -1.0f, -1.0f},
    .front = {2.0f, 0.0f, 0.0f},
    .up = {0.0f, 1.0f, 0.0f},
    .speed = 1,
    .sensitivity = 6,
};

static void sgTransformCamera(const SgCameraTransformInfo* pTransformInfo, SgCamera* pCamera) {

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

static void keyPressCallback(GLFWwindow *pWindow, SgCamera *pCamera) {
	int state;
	v3 dir = {0.0f};
	state = glfwGetKey(pWindow, FOWARD_KEY);
	if (state == GLFW_PRESS)
		dir[0] += 1.0f;
	state = glfwGetKey(pWindow, LEFT_KEY);
	if (state == GLFW_PRESS)
		dir[2] -= 1.0f;
	state = glfwGetKey(pWindow, BACK_KEY);
	if (state == GLFW_PRESS)
		dir[0] -= 1.0f;
	state = glfwGetKey(pWindow, RIGHT_KEY);
	if (state == GLFW_PRESS)
		dir[2] += 1.0f;
	state = glfwGetKey(pWindow, UP_KEY);
	if (state == GLFW_PRESS)
		dir[1] += 1.0f;
	state = glfwGetKey(pWindow, DOWN_KEY);
	if (state == GLFW_PRESS)
		dir[1] -= 1.0f;
	SgCameraTransformInfo transformInfo = {
		.moveDirection = {dir[0], dir[1], dir[2]},
		.deltaTime = scene.deltaTime,
	};
	sgTransformCamera(&transformInfo, &camera);
}

static void cursorPositionCallback(GLFWwindow *pWindow, double xPosition, double yPosition) {
	int wsizex, wsizey;
	glfwGetWindowSize(pWindow, &wsizex, &wsizey);
	if (xPosition == wsizex/2 && yPosition == wsizey/2) {
		camera.cursorPosition[0] = xPosition;
		camera.cursorPosition[1] = yPosition;
		return;
	}
	SgCameraTransformInfo transformInfo = {
		.cursorOffset = { xPosition - camera.cursorPosition[0], camera.cursorPosition[1] - yPosition },
		.deltaTime = scene.deltaTime,
	};
	sgTransformCamera(&transformInfo, &camera);
	camera.cursorPosition[0] = xPosition;
	camera.cursorPosition[1] = yPosition;
}


int main() {
	SgAppCreateInfo createInfo = {
		.pName = "Space Invaders",
		.size  = {640, 480},
		.flags = SG_APP_CURSOR_HIDDEN,
//		.flags = SG_APP_WINDOW_FULLSCREEN,
	};

	SgApp app;

	sgCreateApp(&createInfo, &app);
	sgSetCursorPosCallback(&app, cursorPositionCallback);

	camera.aspectRatio = createInfo.size[0]/createInfo.size[1];
	camera.fov = deg_to_rad(80.f);
	sgSceneInit(&scene);

	// Shaders
	SgFile vertShaderFile;
	SgFile fragShaderFile;
	sgOpenFile("shaders/mesh3.vert.spv", &vertShaderFile);
	sgOpenFile("shaders/mesh3.frag.spv", &fragShaderFile);
	SgShaderCreateInfo vertShaderCreateInfo = {
		.file = vertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo fragShaderCreateInfo = {
		.file = fragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShader vertShader;
	sgCreateShader(app, &vertShaderCreateInfo, &vertShader);
	SgShader fragShader;
	sgCreateShader(app, &fragShaderCreateInfo, &fragShader);
	sgCloseFile(&vertShaderFile);
	sgCloseFile(&fragShaderFile);

	/* Should be inside of an API */
	SgTransformUniform transformuniform = {0};

	sgInitTransformUniform(&camera, &transformuniform);
	/**/
	SgResource cameraResource;
	SgResourceCreateInfo cameraResourceCreateInfo = {
		.binding = 0,
		.bytes = &transformuniform,
		.size = sizeof(SgTransformUniform),
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
		.type = SG_RESOURCE_TYPE_UNIFORM,
	};
	sgCreateResource(app, &cameraResourceCreateInfo, &cameraResource);


	/* Placeholder Mesh */
	SgMesh *pMesh;
	
	sgLoadMesh("res/kitten.obj", &pMesh);
	SgMeshTransformInfo kittenMeshTransformInfo = {
		.move = {1.5, 0.1, 0.1},
		.scale = {2, 2, 2,},
	};
	sgTransformMesh(&kittenMeshTransformInfo, pMesh->vertexCount, pMesh->pVertices);
	/**/

	/* Resource Init */
	SgResource meshResource;
	SgResourceCreateInfo meshResourceCreateInfo = {
		.binding = 0,
		.bytes = pMesh->pVertices,
		.size = pMesh->vertexCount * sizeof(pMesh->pVertices[0]),
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
		.type = SG_RESOURCE_TYPE_MESH,
	};
	sgCreateResource(app, &meshResourceCreateInfo, &meshResource);

	SgResource meshIndicesResource;
	SgResourceCreateInfo meshIndicesResourceCreateInfo = {
		.bytes = pMesh->pIndices,
		.size = pMesh->indexCount * sizeof(pMesh->pIndices[0]),
		.type = SG_RESOURCE_TYPE_INDICES,
	};
	sgCreateResource(app, &meshIndicesResourceCreateInfo, &meshIndicesResource);
	/* Placeholder Mesh2 */
	SgMesh *pMesh2;
	
	sgLoadMesh("res/kitten.obj", &pMesh2);
	SgMeshTransformInfo kittenMeshTransformInfo2 = {
		.move = {.5, 0.1, 0.1},
		.scale = {1, 1, 1,},
	};
	sgTransformMesh(&kittenMeshTransformInfo2, pMesh2->vertexCount, pMesh2->pVertices);
	/**/

	/* Resource Init */
	SgResource meshResource2;
	SgResourceCreateInfo meshResourceCreateInfo2 = {
		.binding = 0,
		.bytes = pMesh2->pVertices,
		.size = pMesh2->vertexCount * sizeof(pMesh2->pVertices[0]),
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
		.type = SG_RESOURCE_TYPE_MESH,
	};
	sgCreateResource(app, &meshResourceCreateInfo2, &meshResource2);

	SgResource meshIndicesResource2;
	SgResourceCreateInfo meshIndicesResourceCreateInfo2 = {
		.bytes = pMesh2->pIndices,
		.size = pMesh2->indexCount * sizeof(pMesh2->pIndices[0]),
		.type = SG_RESOURCE_TYPE_INDICES,
	};
	sgCreateResource(app, &meshIndicesResourceCreateInfo2, &meshIndicesResource2);

	/* Resource Set Init */
	SgResource pMeshSetResources[] = {meshResource};
	SgResourceSetCreateInfo meshResourceSetCreateInfo = {
		.pResources = pMeshSetResources,
		.resourceCount = sizeof(pMeshSetResources) / sizeof(pMeshSetResources[0]),
		.setIndex = 0,
	};
	SgResourceSet meshResourceSet;
	sgCreateResourceSet(app, &meshResourceSetCreateInfo, &meshResourceSet);
	//

	SgResource pMeshSetResources2[] = {meshResource2};
	SgResourceSetCreateInfo meshResourceSetCreateInfo2 = {
		.pResources = pMeshSetResources2,
		.resourceCount = sizeof(pMeshSetResources) / sizeof(pMeshSetResources[0]),
		.setIndex = 0,
	};
	SgResourceSet meshResourceSet2;

	sgCreateResourceSet(app, &meshResourceSetCreateInfo2, &meshResourceSet2);
	//

	SgResource pUniformSetResources[] = {cameraResource};
	SgResourceSetCreateInfo uniformResourceSetCreateInfo = {
		.pResources = pUniformSetResources,
		.resourceCount = sizeof(pUniformSetResources) / sizeof(pUniformSetResources[0]),
		.setIndex = 1,
	};
	SgResourceSet uniformResourceSet;
	sgCreateResourceSet(app, &uniformResourceSetCreateInfo, &uniformResourceSet);

	/* Graphics Instance Init */
	SgShader pShaders[] = {vertShader, fragShader};
	SgResourceSet pResourceSets[] = {uniformResourceSet};
	SgResourceSet pMeshResourceSets[] = {meshResourceSet, meshResourceSet2};
	SgGraphicsInstanceCreateInfo graphicsInstanceCreateInfo = {
		.pShaders = pShaders,
		.shaderCount = sizeof(pShaders) / sizeof(pShaders[0]),

		.pSets = pResourceSets,
		.setCount = sizeof(pResourceSets) / sizeof(pResourceSets[0]),

		.pMeshSets = pMeshResourceSets,
		.meshSetCount = sizeof(pMeshResourceSets) / sizeof(pMeshResourceSets[0]),
	};
	SgGraphicsInstance graphicsInstance;
	sgCreateGraphicsInstance(app, &graphicsInstanceCreateInfo, &graphicsInstance);

	SgResourceSetInitInfo uniformSetInitInfo = {
		.graphicsInstance = graphicsInstance,
		.pResources = pUniformSetResources,
		.resourceCount = sizeof(pUniformSetResources)/sizeof(pUniformSetResources[0]),
	};
	sgInitResourceSet(app, &uniformSetInitInfo, &uniformResourceSet);
	SgResourceSetInitInfo meshSetInitInfo = {
		.graphicsInstance = graphicsInstance,
		.pResources = pMeshSetResources,
		.resourceCount = sizeof(pMeshSetResources)/sizeof(pMeshSetResources[0]),

		.isMeshResourceSet = 1,
		.meshResourceSetID = 0,
	};
	sgInitResourceSet(app, &meshSetInitInfo, &meshResourceSet);

	SgResourceSetInitInfo meshSetInitInfo2 = {
		.graphicsInstance = graphicsInstance,
		.pResources = pMeshSetResources2,
		.resourceCount = sizeof(pMeshSetResources2)/sizeof(pMeshSetResources2[0]),

		.isMeshResourceSet = 1,
		.meshResourceSetID = 1,
	};
	sgInitResourceSet(app, &meshSetInitInfo2, &meshResourceSet2);
	
	SgResource pIndexResources[] = {meshIndicesResource, meshIndicesResource2};

	SgUpdateCommandsInitInfo updateInitInfo = {
		.app = app,
		.graphicsInstance = graphicsInstance,
		.pIndexResouces = pIndexResources,
		.indexResourceCount = 2,
	};
	SgUpdateCommands updateCommands;
	sgInitUpdateCommands(&updateInitInfo, &updateCommands);

	SgAppUpdateInfo updateInfo = {
		.app = app,
		.graphicsInstance = graphicsInstance,
		.updateCommands = updateCommands,
	};

	GLFWwindow *pWindow = sgGetGLFWwindow(app);
	while(sgAppUpdate(&updateInfo)) {
		sgSceneUpdate(&scene);
		keyPressCallback(pWindow, &camera);
		sgUpdateTransformUniform(&camera, &transformuniform);
		SgData cameraData = {
			.bytes = &transformuniform,
			.size  = sizeof(transformuniform),
		};
		sgUpdateResource(app, &cameraData, &cameraResource);
	}

	sgDeinitUpdateCommands(app, &updateCommands);
	sgDestroyGraphicsInstance(app, &graphicsInstance);
	sgDestroyApp(&app);

	return 0;
}
