#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_res.h"
#include "sage_scene.h"
#include "sage_input.h"
#include "log.h"
#include "math.h"

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

static void cursorEnterCallback(GLFWwindow* window, int entered) {
    if (entered) {
		int wsizex, wsizey;
		glfwGetWindowSize(window, &wsizex, &wsizey);
		camera.cursorPosition[0] = wsizex/2;
		camera.cursorPosition[1] = wsizey/2;
	}
}

static void rotateCamera(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	int wsizex, wsizey;
	SgCameraTransformInfo* pTransformInfo = (SgCameraTransformInfo*) actor;
	glfwGetWindowSize(pWindow, &wsizex, &wsizey);
	if (rangeX == wsizex/2 && rangeY == wsizey/2) {
		camera.cursorPosition[0] = rangeX;
		camera.cursorPosition[1] = rangeY;
		return;
	}
	pTransformInfo->cursorOffset[0] = rangeX - camera.cursorPosition[0];
	pTransformInfo->cursorOffset[1] = camera.cursorPosition[1] - rangeY;

	camera.cursorPosition[0] = rangeX;
	camera.cursorPosition[1] = rangeY;
}

static void keyPressForward(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[0] = isPressed ? 1.0f : 0.0f;
	return;
}
static void keyPressUp(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[1] = isPressed ? 1.0f : 0.0f;
	return;
}
static void keyPressLeft(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[2] = isPressed ? -1.0f : 0.0f;
	return;
}
static void keyPressRight(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[2] = isPressed ? 1.0f : 0.0f;
	return;
}
static void keyPressDown(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[1] = isPressed ? -1.0f : 0.0f;
	return;
}
static void keyPressBack(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	SgCameraTransformInfo* transformInfo = (SgCameraTransformInfo*) actor;
	transformInfo->moveDirection[0] = isPressed ? -1.0f : 0.0f;
	return;
}

static void keyShoot(SgBool isPressed, double rangeX, double rangeY, SgActor actor, GLFWwindow *pWindow) {
	if (isPressed)
		log_warn("shooty shooty bang bang");
	return;
}

int main() {
	// Testing
	
	SgFile configFile;
	sgOpenFile("cfg/contexts.json", &configFile);

	SgInputAction inputActions[] = {
		keyPressForward,
		keyPressLeft,
		keyPressBack,
		keyPressRight,
		keyPressUp,
		keyPressDown,
		keyShoot,
		rotateCamera,
	};
	char* inputActionNames[] = {
		"moveForward",
		"moveLeft",  
		"moveBack",  
		"moveRight", 
		"moveUp",
		"moveDown",
		"keyShoot",
		"rotateCamera",
	};

	SgCameraTransformInfo cameraTransform = {0};
	SgActor testActors[] = {
		&cameraTransform
	};

	char* actorNames[] = {
		"cameraTransform",
	};

	SgInputSignal inputSignals[] = {
		{
			.key     = GLFW_KEY_I,
			.keyName = "i",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_H,
			.keyName = "h",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_N,
			.keyName = "n",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_L,
			.keyName = "l",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_K,
			.keyName = "k",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_J,
			.keyName = "j",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_W,
			.keyName = "w",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_A,
			.keyName = "a",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_S,
			.keyName = "s",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_D,
			.keyName = "d",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_KEY_SPACE,
			.keyName = "space",
			.type    = SG_INPUT_TYPE_KEYBOARD,
		},
		{
			.key     = GLFW_MOUSE_BUTTON_1,
			.keyName = "left",
			.type    = SG_INPUT_TYPE_MOUSE,
		},
	};

	SgActiveContexts contexts;
	SgActiveContextsCreateInfo activeContextsCreateInfo = {

		.pActionFuncs  = inputActions,
		.pActionNames  = inputActionNames,
		.actionCount   = sizeof(inputActions) / sizeof(*inputActions),

		.pActors       = testActors,
		.pActorNames   = actorNames,
		.actorCount    = sizeof(testActors) / sizeof(*testActors),

		.pInputSignals = inputSignals,
		.signalCount   = sizeof(inputSignals) / sizeof(*inputSignals),

		.file          = configFile,
	};
	sgLoadContexts(&activeContextsCreateInfo, &contexts);
	sgCloseFile(&configFile);

	//
	
	SgAppCreateInfo createInfo = {
		.pName = "Space Invaders",
		.size  = {640, 480},
		.flags = SG_APP_CURSOR_HIDDEN,
//		.flags = SG_APP_WINDOW_FULLSCREEN,
	};

	SgApp app;

	sgCreateApp(&createInfo, &app);

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
	sgSetActiveContexts(contexts, &app);

	while(sgAppUpdate(&updateInfo)) {
		sgSceneUpdate(&scene);
		cameraTransform.deltaTime = scene.deltaTime;
		sgTransformCamera(&cameraTransform, &camera);
		sgUpdateTransformUniform(&camera, &transformuniform);
		cameraTransform.cursorOffset[0] = 0;
		cameraTransform.cursorOffset[1] = 0;
		SgData cameraData = {
			.bytes = &transformuniform,
			.size  = sizeof(transformuniform),
		};
		sgUpdateResource(app, &cameraData, &cameraResource);
	}

	sgDestroyResource(app, &meshResource);
	sgDestroyResource(app, &meshIndicesResource);
	sgDestroyResource(app, &meshResource2);
	sgDestroyResource(app, &meshIndicesResource2);
	sgDestroyResource(app, &cameraResource);
	sgDestroyResourceSet(app, &meshResourceSet);
	sgDestroyResourceSet(app, &meshResourceSet2);
	sgDestroyResourceSet(app, &uniformResourceSet);
	sgDeinitUpdateCommands(app, &updateCommands);
	sgDestroyGraphicsInstance(app, &graphicsInstance);
	sgDestroyShader(app, &vertShader);
	sgDestroyShader(app, &fragShader);
	sgUnloadContexts(app, &contexts); 
	sgDestroyApp(&app);
	sgUnloadMesh(&pMesh);
	sgUnloadMesh(&pMesh2);

	return 0;
}
