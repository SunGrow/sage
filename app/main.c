#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_scene.h"
#include "sage_input.h"
#include "log.h"
#include "math.h"
#include "inputKeys.h"
#include "inputFuncs.h"

#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))

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
	SgMeshSet *pMesh;
	
	sgCreateMeshSet(&pMesh);
	uint32_t kittenMeshID = sgAddMesh("res/kitten.obj", &pMesh);
	uint32_t chaletMeshID = sgAddMesh("res/chalet.obj", &pMesh);
	uint32_t mesh3ID = sgAddMesh("res/cube.obj", &pMesh);
	uint32_t mesh4ID = sgAddMesh("res/myriam.obj", &pMesh);
	SgMeshTransformInfo kittenMeshTransformInfo = {
		.move = {2.5, 0.3, 1.1},
		.scale = {0.5, 0.5, 0.5,},
	};
	sgTransformMesh(&kittenMeshTransformInfo, pMesh->pVertexOffsets[kittenMeshID], pMesh->pVertexSizes[kittenMeshID], pMesh->pVertices);

	SgMeshTransformInfo kittenMeshTransformInfo2 = {
		.move = {.1, 0.1, 1.1},
		.scale = {0.5, 0.5, -0.5,},
	};
	sgTransformMesh(&kittenMeshTransformInfo2, pMesh->pVertexOffsets[chaletMeshID], pMesh->pVertexSizes[chaletMeshID], pMesh->pVertices);

	SgMeshTransformInfo kittenMeshTransformInfo3 = {
		.move = {1.0, -0.1, -0.1},
		.scale = {0.5, 0.5, 0.5,},
	};
	sgTransformMesh(&kittenMeshTransformInfo3, pMesh->pVertexOffsets[mesh3ID], pMesh->pVertexSizes[mesh3ID], pMesh->pVertices);

	SgMeshTransformInfo kittenMeshTransformInfo4 = {
		.move = {0.5, 0.3, -1.1},
		.scale = {0.01, 0.01, 0.01,},
	};
	sgTransformMesh(&kittenMeshTransformInfo4, pMesh->pVertexOffsets[mesh4ID], pMesh->pVertexSizes[mesh4ID], pMesh->pVertices);
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
	// TODO: Wrap into a creation inside of an api, ffs
	pMesh->indicesResource = meshIndicesResource;
	/* Placeholder Mesh2 */
	/**/

	/* Resource Init */
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

	SgMaterialMap materialMap;
	sgCreateMaterialMap(app, 1, &materialMap);

	SgShader pShadersChalet[] = {vertShader, fragShader};
	SgResourceBinding* materialChaletResourceBindings[] = {
		(SgResourceBinding[])
		{
			(SgResourceBinding) {.type = SG_RESOURCE_TYPE_MESH, .stage = SG_SHADER_STAGE_VERTEX_BIT, .binding = 0},
			(SgResourceBinding){.type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .binding = 1},
		},
		(SgResourceBinding[])
		{
			(SgResourceBinding){.type = SG_RESOURCE_TYPE_UNIFORM, .stage = SG_SHADER_STAGE_VERTEX_BIT, .binding = 0},
		},
	};
	SgMaterialCreateInfo materialChaletCreateInfo = {
		.pMaterialName = "materialChalet",
		.pShaders = pShadersChalet,
		.shaderCount = NUMOF(pShadersChalet),
		.resourceSetBindingCount = NUMOF(materialChaletResourceBindings),
		.pResourceBindingCount = (uint32_t[]) {
			2,
			1,
		},
		.ppResourceBindings = (SgResourceBinding**) materialChaletResourceBindings,
	};
	sgAddMaterial(&materialChaletCreateInfo, &materialMap);
	SgRenderObject chaletRenderObjects[] = {
		{
			.meshID = chaletMeshID,
		},
		{
			.meshID = kittenMeshID,
		}
	};
	SgRenderObjectCreateInfo chaletRenderObject = {
		.materialName = "materialChalet",
		.materialObjectsName = "chaletMesh",
		.pRenderObjects = chaletRenderObjects,
		.renderObjectCount = NUMOF(chaletRenderObjects),
		.pResourceSetBindings = (uint32_t[]) {0, 0, 1},
		.pResources = (SgResource[]) {meshResource,  meshTextureResource, cameraResource},
		.resourceCount = 3,
		.resourceSetCount = NUMOF(materialChaletResourceBindings),
	};
	sgAddMaterialRenderObjects(&chaletRenderObject, &materialMap);
	sgInitMaterialMap(app, &materialMap);
	sgWriteMaterialRenderObjects(&materialMap);

	/* Graphics Instance Init */

	SgUpdateCommandsInitInfo updateInitInfo = {
		.materialMap = materialMap,
		.pMeshSet = pMesh,
	};
	SgUpdateCommands updateCommands;
	sgInitUpdateCommands(&updateInitInfo, &updateCommands);

	SgAppUpdateInfo updateInfo = {
		.app = app,
		.materialMap = materialMap,
		.pMeshSet = pMesh,
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
	sgDestroyResource(app, &cameraResource);
	sgDeinitUpdateCommands(app, &updateCommands);
	sgDestroyShader(app, &vertShader);
	sgDestroyShader(app, &fragShader);
	sgUnloadContexts(app, &contexts); 
	sgDestroyApp(&app);
	sgUnloadMesh(&pMesh);
	sgUnloadTexture(&pTexture);
	sgUnloadTexture(&pTexture2);

	return 0;
}
