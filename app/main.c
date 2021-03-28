#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_res.h"
#include "sage_scene.h"
#include "sage_input.h"
#include "log.h"
#include "math.h"
#include "inputKeys.h"
#include "inputFuncs.h"


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

	SgCamera camera = {
	    .position = {-2.0f, -1.0f, -1.0f},
	    .front = {2.0f, 0.0f, 0.0f},
	    .up = {0.0f, 1.0f, 0.0f},
	    .speed = 1,
	    .sensitivity = 6,
	};

	SgCameraTransformInfo cameraTransform = {0};
	cameraTransform.camera = camera;

	SgScene scene = {0};

	SgActor testActors[] = {
		&cameraTransform
	};

	char* actorNames[] = {
		"cameraTransform",
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
	//sgCloseFile(&configFile);
	//SgActionNames oldNames[] = {
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "k",
	//		.modName = "",
	//		.actionName = "moveUp",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "i",
	//		.modName = "",
	//		.actionName = "moveForward",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "n",
	//		.modName = "",
	//		.actionName = "moveBack",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "l",
	//		.modName = "",
	//		.actionName = "moveRight",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "h",
	//		.modName = "",
	//		.actionName = "moveLeft",
	//	},
	//};
	//SgActionNames newNames[] = {
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "space",
	//		.modName = "",
	//		.actionName = "moveUp",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "w",
	//		.modName = "",
	//		.actionName = "moveForward",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "s",
	//		.modName = "",
	//		.actionName = "moveBack",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "d",
	//		.modName = "",
	//		.actionName = "moveRight",
	//	},
	//	{
	//		.actionType = SG_ACTION_TYPE_TRIGGER,
	//		.inputType = SG_INPUT_TYPE_KEYBOARD,
	//		.inputName = "a",
	//		.modName = "",
	//		.actionName = "moveLeft",
	//	},
	//};
	//SgActiveContextsChangeInfo changeInfo = {
	//	.pOldActions = oldNames,
	//	.pNewActions = newNames,
	//	.count = sizeof(oldNames) / sizeof(*oldNames),
	//};
	//sgChangeContext(&changeInfo, &contexts);
	//sgUpdateContext(&activeContextsCreateInfo, &contexts);
	//sgSaveContext(contexts, "cfg/contexts.json");
	//
	

	SgAppCreateInfo createInfo = {
		.pName = "Space Invaders",
		.size  = {640, 480},
		.flags = SG_APP_CURSOR_HIDDEN,
//		.flags = SG_APP_WINDOW_FULLSCREEN,
	};

	SgApp app;

	sgCreateApp(&createInfo, &app);

	cameraTransform.camera.aspectRatio = createInfo.size[0]/createInfo.size[1];
	cameraTransform.camera.fov = deg_to_rad(80.f);
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

	sgInitTransformUniform(&cameraTransform.camera, &transformuniform);
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
	
	sgLoadMesh("res/chalet.obj", &pMesh);
	SgMeshTransformInfo kittenMeshTransformInfo = {
		.move = {2.5, 0.3, 1.1},
		.scale = {1, 1, 1,},
	};
	sgTransformMesh(&kittenMeshTransformInfo, 0, pMesh->vertexCount, pMesh->pVertices);

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

	SgTexture *pTexture;
	sgLoadTexture("res/chalet.jpg", &pTexture);
	SgResource meshTextureResource;
	SgResourceCreateInfo meshTextureResourceCreateInfo = {
		.binding = 1,
		.bytes = pTexture->pixels,
		.size = pTexture->size,
		.extent = (VkExtent3D) {.height = pTexture->height, .width = pTexture->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &meshTextureResourceCreateInfo, &meshTextureResource);
	/* Placeholder Mesh2 */
	SgMesh *pMesh2;
	
	sgLoadMesh("res/myriam.obj", &pMesh2);
	SgMeshTransformInfo kittenMeshTransformInfo2 = {
		.move = {.5, 0.1, 0.1},
		.scale = {0.01, 0.01, 0.01,},
	};
	sgTransformMesh(&kittenMeshTransformInfo2, 0, pMesh2->vertexCount, pMesh2->pVertices);
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
	SgTexture *pTexture2;
	sgLoadTexture("res/skin.jpg", &pTexture2);
	SgResource meshTexture2Resource;
	SgResourceCreateInfo meshTexture2ResourceCreateInfo = {
		.binding = 1,
		.bytes = pTexture2->pixels,
		.size = pTexture2->size,
		.extent = (VkExtent3D) {.height = pTexture2->height, .width = pTexture2->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &meshTexture2ResourceCreateInfo, &meshTexture2Resource);

	/* Resource Set Init */
	SgResource pMeshSetResources[] = {meshResource, meshTextureResource};
	SgResourceSetCreateInfo meshResourceSetCreateInfo = {
		.pResources = pMeshSetResources,
		.resourceCount = sizeof(pMeshSetResources) / sizeof(pMeshSetResources[0]),
		.setIndex = 0,
	};
	SgResourceSet meshResourceSet;
	sgCreateResourceSet(app, &meshResourceSetCreateInfo, &meshResourceSet);
	//

	SgResource pMeshSetResources2[] = {meshResource2, meshTexture2Resource};
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
		sgTransformCamera(&cameraTransform, &cameraTransform.camera);
		sgUpdateTransformUniform(&cameraTransform.camera, &transformuniform);
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
	sgDestroyResource(app, &meshTextureResource);
	sgDestroyResource(app, &meshTexture2Resource);
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
	sgUnloadTexture(&pTexture);
	sgUnloadTexture(&pTexture2);

	return 0;
}
