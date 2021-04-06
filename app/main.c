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
	SgFile chaletVertShaderFile;
	SgFile chaletFragShaderFile;
	sgOpenFile("shaders/chaletShader.vert.spv", &chaletVertShaderFile);
	sgOpenFile("shaders/chaletShader.frag.spv", &chaletFragShaderFile);
	SgShaderCreateInfo chaletVertShaderCreateInfo = {
		.file = chaletVertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo chaletFragShaderCreateInfo = {
		.file = chaletFragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShader chaletVertShader;
	sgCreateShader(app, &chaletVertShaderCreateInfo, &chaletVertShader);
	SgShader chaletFragShader;
	sgCreateShader(app, &chaletFragShaderCreateInfo, &chaletFragShader);
	sgCloseFile(&chaletVertShaderFile);
	sgCloseFile(&chaletFragShaderFile);

	// Shaders
	SgFile myriamVertShaderFile;
	SgFile myriamFragShaderFile;
	sgOpenFile("shaders/myriamShader.vert.spv", &myriamVertShaderFile);
	sgOpenFile("shaders/myriamShader.frag.spv", &myriamFragShaderFile);
	SgShaderCreateInfo myriamVertShaderCreateInfo = {
		.file = myriamVertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo myriamFragShaderCreateInfo = {
		.file = myriamFragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShader myriamVertShader;
	sgCreateShader(app, &myriamVertShaderCreateInfo, &myriamVertShader);
	SgShader myriamFragShader;
	sgCreateShader(app, &myriamFragShaderCreateInfo, &myriamFragShader);
	sgCloseFile(&myriamVertShaderFile);
	sgCloseFile(&myriamFragShaderFile);

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
	uint32_t cubeMeshID   = sgAddMesh("res/cube.obj", &pMesh);
	uint32_t mytiamMeshID = sgAddMesh("res/myriam.obj", &pMesh);
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
	sgTransformMesh(&kittenMeshTransformInfo3, pMesh->pVertexOffsets[cubeMeshID], pMesh->pVertexSizes[cubeMeshID], pMesh->pVertices);

	SgMeshTransformInfo kittenMeshTransformInfo4 = {
		.move = {0.5, 0.3, -1.1},
		.scale = {0.01, 0.01, 0.01,},
	};
	sgTransformMesh(&kittenMeshTransformInfo4, pMesh->pVertexOffsets[mytiamMeshID], pMesh->pVertexSizes[mytiamMeshID], pMesh->pVertices);
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
	SgTexture *pChaletTexture;
	sgLoadTexture("res/chalet.jpg", &pChaletTexture);
	SgResource meshTextureResource;
	SgResourceCreateInfo meshTextureResourceCreateInfo = {
		.binding = 1,
		.bytes = pChaletTexture->pixels,
		.size = pChaletTexture->size,
		.extent = (VkExtent3D) {.height = pChaletTexture->height, .width = pChaletTexture->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &meshTextureResourceCreateInfo, &meshTextureResource);

	///
	SgTexture *pSkinTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_01Skin.jpg", &pSkinTexture);
	SgResource skinTextureResource;
	SgResourceCreateInfo skinTextureResourceCreateInfo = {
		.binding = 1,
		.bytes = pSkinTexture->pixels,
		.size = pSkinTexture->size,
		.extent = (VkExtent3D) {.height = pSkinTexture->height, .width = pSkinTexture->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &skinTextureResourceCreateInfo, &skinTextureResource);

	SgTexture *pHairTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_02Hair.jpg", &pHairTexture);
	SgResource hairTextureResource;
	SgResourceCreateInfo hairTextureResourceCreateInfo = {
		.binding = 2,
		.bytes = pHairTexture->pixels,
		.size = pHairTexture->size,
		.extent = (VkExtent3D) {.height = pHairTexture->height, .width = pHairTexture->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &hairTextureResourceCreateInfo, &hairTextureResource);

	SgTexture *pClothTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_03Cloth.jpg", &pClothTexture);
	SgResource clothTextureResource;
	SgResourceCreateInfo clothTextureResourceCreateInfo = {
		.binding = 3,
		.bytes = pClothTexture->pixels,
		.size = pClothTexture->size,
		.extent = (VkExtent3D) {.height = pClothTexture->height, .width = pClothTexture->width, .depth = 1.0},
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,
	};
	sgCreateResource(app, &clothTextureResourceCreateInfo, &clothTextureResource);
	///

	SgMaterialMap materialMap;
	sgCreateMaterialMap(app, 1, &materialMap);

	SgShader pShadersChalet[] = {chaletVertShader, chaletFragShader};
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

	SgShader pShadersMyriam[] = {myriamVertShader, myriamFragShader};
	SgResourceBinding materialMyriamSet1ResourceBinding[] = {
		(SgResourceBinding){.type = SG_RESOURCE_TYPE_MESH, .stage = SG_SHADER_STAGE_VERTEX_BIT, .binding = 0},
		(SgResourceBinding){.type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .binding = 1},
		(SgResourceBinding){.type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .binding = 2},
		(SgResourceBinding){.type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .binding = 3},
	};
	SgResourceBinding materialMyriamSet2ResourceBinding[] = {
		(SgResourceBinding){.type = SG_RESOURCE_TYPE_UNIFORM, .stage = SG_SHADER_STAGE_VERTEX_BIT, .binding = 0},
	};
	SgResourceBinding* materialMyriamResourceBindings[] = {
		materialMyriamSet1ResourceBinding,
		materialMyriamSet2ResourceBinding,
	};
	SgMaterialCreateInfo materialMyriamCreateInfo = {
		.pMaterialName = "materialMyriam",
		.pShaders = pShadersMyriam,
		.shaderCount = NUMOF(pShadersMyriam),
		.resourceSetBindingCount = NUMOF(materialMyriamResourceBindings),
		.pResourceBindingCount = (uint32_t[]) {
			NUMOF(materialMyriamSet1ResourceBinding),
			NUMOF(materialMyriamSet2ResourceBinding),
		},
		.ppResourceBindings = (SgResourceBinding**) materialMyriamResourceBindings,
	};
	sgAddMaterial(&materialMyriamCreateInfo, &materialMap);


	SgRenderObject myriamRenderObjects[] = {
		{
			.meshID = mytiamMeshID,
		},
		{
			.meshID = cubeMeshID,
		},
	};
	SgResource myriamRenderObjectResources[] = {
		meshResource,
		/*TODO: texture arrays */
		skinTextureResource,
		hairTextureResource,
		clothTextureResource,
		/**/
		cameraResource,
	};
	// TODO: move set bindings into a resource layout
	SgRenderObjectCreateInfo myriamRenderObject = {
		.materialName = "materialMyriam",
		.materialObjectsName = "myriamMesh",
		.pRenderObjects = myriamRenderObjects,
		.renderObjectCount = NUMOF(myriamRenderObjects),
		.pResourceSetBindings = (uint32_t[]) {0, 0, 0, 0, 1},
		.pResources = myriamRenderObjectResources,
		.resourceCount = NUMOF(myriamRenderObjectResources),
		.resourceSetCount = NUMOF(materialMyriamResourceBindings),
	};
	sgAddMaterialRenderObjects(&myriamRenderObject, &materialMap);

	SgRenderObject chaletRenderObjects[] = {
		{
			.meshID = chaletMeshID,
		},
		{
			.meshID = kittenMeshID,
		},
	};
	// TODO: render object to have descriptor sets instead of materials
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

	// TODO: Proper cleanup

	sgDestroyResource(app, &meshResource);
	sgDestroyResource(app, &meshIndicesResource);
	sgDestroyResource(app, &meshTextureResource);
	sgDestroyResource(app, &skinTextureResource);
	sgDestroyResource(app, &cameraResource);
	sgDeinitUpdateCommands(app, &updateCommands);
	sgDestroyShader(app, &chaletVertShader);
	sgDestroyShader(app, &chaletFragShader);
	sgUnloadContexts(app, &contexts); 
	sgDestroyApp(&app);
	sgUnloadMesh(&pMesh);
	sgUnloadTexture(&pChaletTexture);
	sgUnloadTexture(&pSkinTexture);

	return 0;
}
