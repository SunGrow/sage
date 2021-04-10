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
	SgResourceMap resourceMap;
	sgCreateResourceMap(app, &resourceMap);
	SgResourceCreateInfo cameraResourceCreateInfo = {
		.bytes = &transformuniform,
		.size = sizeof(SgTransformUniform),
		.type = SG_RESOURCE_TYPE_UNIFORM,
		.pName = "cameraResource",
	};
	sgAddResource(app, &cameraResourceCreateInfo, &resourceMap);


	/* Placeholder Mesh */
	SgMeshSet *pMesh;
	
	sgCreateMeshSet(&pMesh);
	uint32_t kittenMeshID = sgAddMesh("res/kitten.obj", &pMesh);
	uint32_t chaletMeshID = sgAddMesh("res/chalet.obj", &pMesh);
	uint32_t cubeMeshID   = sgAddMesh("res/cube.obj", &pMesh);
	uint32_t myriamMeshID = sgAddMesh("res/myriam.obj", &pMesh);
	/**/

	/* Resource Init */
	SgResourceCreateInfo meshResourceCreateInfo = {
		.bytes = pMesh->pVertices,
		.size = pMesh->vertexCount * sizeof(pMesh->pVertices[0]),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "meshResource"
	};
	sgAddResource(app, &meshResourceCreateInfo, &resourceMap);

	SgResourceCreateInfo meshIndicesResourceCreateInfo = {
		.bytes = pMesh->pIndices,
		.size = pMesh->indexCount * sizeof(pMesh->pIndices[0]),
		.type = SG_RESOURCE_TYPE_INDICES,

		.pName = "meshIndicesResource",
	};
	sgAddResource(app, &meshIndicesResourceCreateInfo, &resourceMap);
	// TODO: Wrap into a creation inside of an api, ffs
	pMesh->indicesResourceName = meshIndicesResourceCreateInfo.pName;
	/* Placeholder Mesh2 */
	/**/

	/* Resource Init */
	SgTexture *pChaletTexture;
	sgLoadTexture("res/chalet.jpg", &pChaletTexture);
	SgResourceCreateInfo meshTextureResourceCreateInfo = {
		.bytes = pChaletTexture->pixels,
		.size = pChaletTexture->size,
		.extent = (VkExtent3D) {.height = pChaletTexture->height, .width = pChaletTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "chaletTextureResource",
	};
	sgAddResource(app, &meshTextureResourceCreateInfo, &resourceMap);

	///
	SgTexture *pSkinAlphaTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_01Skin.jpg", &pSkinAlphaTexture);
	SgResourceCreateInfo skinTextureResourceCreateInfo = {
		.bytes = pSkinAlphaTexture->pixels,
		.size = pSkinAlphaTexture->size,
		.extent = (VkExtent3D) {.height = pSkinAlphaTexture->height, .width = pSkinAlphaTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "myriamAlphaSkinTextureResource",
	};
	sgAddResource(app, &skinTextureResourceCreateInfo, &resourceMap);

	SgTexture *pHairAlphaTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_02Hair.jpg", &pHairAlphaTexture);
	SgResourceCreateInfo hairTextureResourceCreateInfo = {
		.bytes = pHairAlphaTexture->pixels,
		.size = pHairAlphaTexture->size,
		.extent = (VkExtent3D) {.height = pHairAlphaTexture->height, .width = pHairAlphaTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "myriamAlphaHairTextureResource",
	};
	sgAddResource(app, &hairTextureResourceCreateInfo, &resourceMap);

	SgTexture *pClothAlphaTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_A_03Cloth.jpg", &pClothAlphaTexture);
	SgResourceCreateInfo clothTextureResourceCreateInfo = {
		.bytes = pClothAlphaTexture->pixels,
		.size = pClothAlphaTexture->size,
		.extent = (VkExtent3D) {.height = pClothAlphaTexture->height, .width = pClothAlphaTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "myriamAlphaClothTextureResource",
	};
	sgAddResource(app, &clothTextureResourceCreateInfo, &resourceMap);

	SgTexture *pBodyDTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_D_2k.jpg", &pBodyDTexture);
	SgResourceCreateInfo bodyDTextureResourceCreateInfo = {
		.bytes = pBodyDTexture->pixels,
		.size = pBodyDTexture->size,
		.extent = (VkExtent3D) {.height = pBodyDTexture->height, .width = pBodyDTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "myriamBodyDTextureResource",
	};
	sgAddResource(app, &bodyDTextureResourceCreateInfo, &resourceMap);
	///

	SgMaterialMap materialMap;
	SgMaterialMapCreateInfo materialMapCreateInfo = {
		.materailCount = 1,
		.resourceMap = resourceMap,
	};
	sgCreateMaterialMap(app, &materialMapCreateInfo, &materialMap);

	SgShader pShadersChalet[] = {chaletVertShader, chaletFragShader};
	SgResourceBinding materialChaletResourceBindings[] = {
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_MESH,       .stage = SG_SHADER_STAGE_VERTEX_BIT,   .setBinding = 0, .binding = 0, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .setBinding = 0, .binding = 1, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_MESH,       .stage = SG_SHADER_STAGE_VERTEX_BIT,   .setBinding = 0, .binding = 2, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_UNIFORM,    .stage = SG_SHADER_STAGE_VERTEX_BIT,   .setBinding = 1, .binding = 0, },
	};
	SgMaterialCreateInfo materialChaletCreateInfo = {
		.pMaterialName = "materialChalet",
		.pShaders = pShadersChalet,
		.shaderCount = NUMOF(pShadersChalet),
		.resourceBindingCount = NUMOF(materialChaletResourceBindings),
		.pResourceBindings = materialChaletResourceBindings,
		.renderObjectCount = 2,
	};
	sgAddMaterial(&materialChaletCreateInfo, &materialMap);

	SgShader pShadersMyriam[] = {myriamVertShader, myriamFragShader};
	SgResourceBinding materialMyriamSetResourceBinding[] = {
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_MESH,       .stage = SG_SHADER_STAGE_VERTEX_BIT,   .setBinding = 0, .binding = 0, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .setBinding = 0, .binding = 1, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .setBinding = 0, .binding = 2, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .setBinding = 0, .binding = 3, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_TEXTURE_2D, .stage = SG_SHADER_STAGE_FRAGMENT_BIT, .setBinding = 0, .binding = 4, },
		(SgResourceBinding){ .type = SG_RESOURCE_TYPE_UNIFORM,    .stage = SG_SHADER_STAGE_VERTEX_BIT,   .setBinding = 1, .binding = 0, },
	};
	SgMaterialCreateInfo materialMyriamCreateInfo = {
		.pMaterialName = "materialMyriam",
		.pShaders = pShadersMyriam,
		.shaderCount = NUMOF(pShadersMyriam),
		.resourceBindingCount = NUMOF(materialMyriamSetResourceBinding),
		.pResourceBindings = materialMyriamSetResourceBinding,
		.renderObjectCount = 1,
	};
	sgAddMaterial(&materialMyriamCreateInfo, &materialMap);
	sgInitMaterialMap(app, &materialMap);


	SgRenderObject pMyriamRenderObjects[] = {
		{
			.meshID = myriamMeshID,
			.instanceCount = 1,
		},
		{
			.meshID = cubeMeshID,
			.instanceCount = 1,
		},
	};
	const char* pMyriamRenderObjectResourceNames[] = {
		meshResourceCreateInfo.pName,
		/*TODO: texture arrays */
		skinTextureResourceCreateInfo.pName,
		hairTextureResourceCreateInfo.pName,
		clothTextureResourceCreateInfo.pName,
		bodyDTextureResourceCreateInfo.pName,
		/**/
		cameraResourceCreateInfo.pName,
	};
	SgRenderObjectCreateInfo myriamRenderObject = {
		.materialName = "materialMyriam",
		.pName = "myriamMesh",
		.pRenderObjects = pMyriamRenderObjects,
		.renderObjectCount = NUMOF(pMyriamRenderObjects),
		.ppResourceNames = pMyriamRenderObjectResourceNames,
		.resourceCount = NUMOF(pMyriamRenderObjectResourceNames),
	};
	sgAddMaterialRenderObjects(&myriamRenderObject, &materialMap);

	struct SgObjectInfo {
		m4 transform;
		v4 position;
	};
	struct SgObjectInfo chaletObjects[] = {
		{
			.transform = {
				{1.0,0.0,0.0,0.0},
				{0.0,1.0,0.0,0.0},
				{0.0,0.0,1.0,0.0},
				{0.0,0.0,0.0,1.0}
			},
			.position = {
				1.0,1.0,0.0,1.0
			},
		},
		{
			.transform = {
				{1.0,0.0,0.0,0.0},
				{0.0,1.0,0.0,0.0},
				{0.0,0.0,1.0,0.0},
				{0.0,0.0,0.0,1.0}
			},
			.position = {
				2.0,0.0,0.0,1.0
			},
		},
	};
	SgRenderObject chaletRenderObjects[] = {
		{
			.meshID = chaletMeshID,
			.instanceCount = NUMOF(chaletObjects),
		},
	};

	SgResourceCreateInfo chaletTransformMatricesCreateInfo = {
		.bytes = chaletObjects,
		.size = sizeof(chaletObjects),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "chaletTransformMatrices",
	};
	sgAddResource(app, &chaletTransformMatricesCreateInfo, &resourceMap);
	const char* materialChaletResourceNames[] = {
		meshResourceCreateInfo.pName, meshTextureResourceCreateInfo.pName, chaletTransformMatricesCreateInfo.pName, cameraResourceCreateInfo.pName
	};
	SgRenderObjectCreateInfo chaletRenderObject = {
		.materialName = "materialChalet",
		.pName = "chaletMesh",
		.pRenderObjects = chaletRenderObjects,
		.renderObjectCount = NUMOF(chaletRenderObjects),
		.ppResourceNames = materialChaletResourceNames,
		.resourceCount = NUMOF(materialChaletResourceNames),
	};
	sgAddMaterialRenderObjects(&chaletRenderObject, &materialMap);

	struct SgObjectInfo kittenObjects[] = {
		{
			.transform = {
				{1.0,0.0,0.0,0.0},
				{0.0,1.0,0.0,0.0},
				{0.0,0.0,1.0,0.0},
				{0.0,0.0,0.0,1.0}
			},
			.position = {
				1.0,2.0,0.0,1.0
			},
		},
		{
			.transform = {
				{1.0,0.0,0.0,0.0},
				{0.0,1.0,0.0,0.0},
				{0.0,0.0,1.0,0.0},
				{0.0,0.0,0.0,1.0}
			},
			.position = {
				0.0,1.0,1.0,1.0
			},
		},
	};

	SgResourceCreateInfo kittenTransformMatricesCreateInfo = {
		.bytes = kittenObjects,
		.size = sizeof(kittenObjects),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "kittenTransformMatrices",
	};
	sgAddResource(app, &kittenTransformMatricesCreateInfo, &resourceMap);
	sgInitResourceMap(app, &resourceMap);
	sgUpdateAllResources(app, resourceMap);
	
	const char* materialKittenResourceNames[] = {
		meshResourceCreateInfo.pName, meshTextureResourceCreateInfo.pName, kittenTransformMatricesCreateInfo.pName, cameraResourceCreateInfo.pName
	};
	SgRenderObject kittenRenderObjects[] = {
		{
			.meshID = kittenMeshID,
			.instanceCount = NUMOF(kittenObjects),
		},
	};
	SgRenderObjectCreateInfo kittenRenderObject = {
		.materialName = "materialChalet",
		.pName = "kittenMesh",
		.pRenderObjects = kittenRenderObjects,
		.renderObjectCount = NUMOF(kittenRenderObjects),
		.ppResourceNames = materialKittenResourceNames,
		.resourceCount = NUMOF(materialKittenResourceNames),
	};
	sgAddMaterialRenderObjects(&kittenRenderObject, &materialMap);

	sgWriteMaterialRenderObjects(&materialMap);

	/* Graphics Instance Init */

	SgUpdateCommandsInitInfo updateInitInfo = {
		.materialMap = materialMap,
		.pMeshSet = pMesh,
		.resourceMap = resourceMap,
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
		sgUpdateResource(app, resourceMap, &cameraData, cameraResourceCreateInfo.pName);
	}

	sgUnloadTexture(&pChaletTexture);
	sgUnloadTexture(&pSkinAlphaTexture);
	sgUnloadTexture(&pHairAlphaTexture);
	sgUnloadTexture(&pClothAlphaTexture);
	sgUnloadTexture(&pBodyDTexture);
	sgDeinitUpdateCommands(app, &updateCommands);

	sgDestroyResourceMap(app, &resourceMap);
	sgDestroyMaterialMap(app, &materialMap);
	sgDestroyShader(app, &chaletVertShader);
	sgDestroyShader(app, &chaletFragShader);
	sgDestroyShader(app, &myriamVertShader);
	sgDestroyShader(app, &myriamFragShader);
	sgUnloadContexts(app, &contexts); 
	sgDestroyApp(&app);
	sgUnloadMesh(&pMesh);

	return 0;
}
