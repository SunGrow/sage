#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_scene.h"
#include "sage_input.h"
#include "log.h"
#include "math.h"
#include "resourceBindings.h"
#include "transformObjects.h"
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

        lightKeyPressFroward,
        lightKeyPressUp,
        lightKeyPressLeft,
        lightKeyPressRight,
        lightKeyPressDown,
        lightKeyPressBack,
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

        "lightForward",
        "lightUp",
        "lightLeft",
        "lightRight",
        "lightDown",
        "lightBack",
	};

	struct SgLighting {
		v4 position;
		v4 color;
	};
	struct SgLighting lighting = {
		.position = {-3.0, 1.0, 0.0, 1.0},
		.color = {1.0,1.0,0.6,1.0},
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
		&cameraTransform,
		&lighting,
	};

	char* actorNames[] = {
		"cameraTransform",
		"lighting",
	};

	SgActiveContexts contexts;
	SgActiveContextsCreateInfo activeContextsCreateInfo = {

		.pActionFuncs  = inputActions,
		.pActionNames  = inputActionNames,
		.actionCount   = NUMOF(inputActions),

		.pActors       = testActors,
		.pActorNames   = actorNames,
		.actorCount    = NUMOF(testActors),

		.pInputSignals = inputSignals,
		.signalCount   = NUMOF(inputSignals),

		.file          = configFile,
	};

	sgLoadContexts(&activeContextsCreateInfo, &contexts);
	sgCloseFile(&configFile);
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
	// TODO: Shader Map
	SgFile chaletVertShaderFile;
	SgFile chaletFragShaderFile;
	SgFile myriamVertShaderFile;
	SgFile myriamFragShaderFile;
	SgFile lightingVertShaderFile;
	SgFile lightingFragShaderFile;
	sgOpenFile("shaders/chaletShader.vert.spv", &chaletVertShaderFile);
	sgOpenFile("shaders/chaletShader.frag.spv", &chaletFragShaderFile);
	sgOpenFile("shaders/myriamShader.vert.spv", &myriamVertShaderFile);
	sgOpenFile("shaders/myriamShader.frag.spv", &myriamFragShaderFile);
	sgOpenFile("shaders/lightSourceShader.vert.spv", &lightingVertShaderFile);
	sgOpenFile("shaders/lightSourceShader.frag.spv", &lightingFragShaderFile);
	SgShaderCreateInfo chaletVertShaderCreateInfo = {
		.file = chaletVertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo chaletFragShaderCreateInfo = {
		.file = chaletFragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShaderCreateInfo myriamVertShaderCreateInfo = {
		.file = myriamVertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo myriamFragShaderCreateInfo = {
		.file = myriamFragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShaderCreateInfo lightingVertShaderCreateInfo = {
		.file = lightingVertShaderFile,
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo lightingFragShaderCreateInfo = {
		.file = lightingFragShaderFile,
		.stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShader chaletVertShader;
	SgShader chaletFragShader;
	SgShader myriamVertShader;
	SgShader myriamFragShader;
	SgShader lightingVertShader;
	SgShader lightingFragShader;

	sgCreateShader(app, &chaletVertShaderCreateInfo, &chaletVertShader);
	sgCreateShader(app, &chaletFragShaderCreateInfo, &chaletFragShader);
	sgCreateShader(app, &myriamVertShaderCreateInfo, &myriamVertShader);
	sgCreateShader(app, &myriamFragShaderCreateInfo, &myriamFragShader);
	sgCreateShader(app, &lightingVertShaderCreateInfo, &lightingVertShader);
	sgCreateShader(app, &lightingFragShaderCreateInfo, &lightingFragShader);

	sgCloseFile(&chaletVertShaderFile);
	sgCloseFile(&chaletFragShaderFile);
	sgCloseFile(&myriamVertShaderFile);
	sgCloseFile(&myriamFragShaderFile);
	sgCloseFile(&lightingVertShaderFile);
	sgCloseFile(&lightingFragShaderFile);
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
	uint32_t myriamMeshID = sgAddMesh("res/myriam.obj", &pMesh);
	uint32_t sphereMeshID = sgAddMesh("res/sphere.obj", &pMesh);
	// Each segment has the same length as the previous one and could be replaced
	// SgSubmeshCreateInfo submeshCreateInfo = {
	//     .submeshSegmentCount = 2,
	//     .submeshSegmentMeshData = (struct SgSubmeshData) {
	//	       .pVertices = &vertices,
	//	       .pIndices  = &indices,
	//	       .indexCount = NUMOF(indices),
	//	       .vertexCount = NUMOF(vertices),
	//     },
	// };
	// sgAddSubmesh(&submeshCreateInfo, &pMesh);
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
	SgTexture *pNormalTexture;
	sgLoadTexture("res/tex/10016_w_Myriam_Body_N_2k.jpg", &pNormalTexture);
	SgResourceCreateInfo normalTextureResourceCreateInfo = {
		.bytes = pNormalTexture->pixels,
		.size = pNormalTexture->size,
		.extent = (VkExtent3D) {.height = pNormalTexture->height, .width = pNormalTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "myriamNormalTextureResource",
	};
	sgAddResource(app, &normalTextureResourceCreateInfo, &resourceMap);


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

	SgTexture *pDefTexture;
	sgLoadTexture("res/def.jpg", &pDefTexture);
	SgResourceCreateInfo defTextureResourceCreateInfo = {
		.bytes = pDefTexture->pixels,
		.size = pDefTexture->size,
		.extent = (VkExtent3D) {.height = pDefTexture->height, .width = pDefTexture->width, .depth = 1.0},
		.type = SG_RESOURCE_TYPE_TEXTURE_2D,

		.pName = "DefTextureResource",
	};
	sgAddResource(app, &defTextureResourceCreateInfo, &resourceMap);
	///

	SgMaterialMap materialMap;
	SgMaterialMapCreateInfo materialMapCreateInfo = {
		.materailCount = 2,
		.resourceMap = resourceMap,
	};
	// Materials
	sgCreateMaterialMap(app, &materialMapCreateInfo, &materialMap);

	SgShader pShadersChalet[] = {chaletVertShader, chaletFragShader};
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
	SgMaterialCreateInfo materialMyriamCreateInfo = {
		.pMaterialName = "materialMyriam",
		.pShaders = pShadersMyriam,
		.shaderCount = NUMOF(pShadersMyriam),
		.resourceBindingCount = NUMOF(materialMyriamSetResourceBinding),
		.pResourceBindings = materialMyriamSetResourceBinding,
		.renderObjectCount = 1,
	};
	sgAddMaterial(&materialMyriamCreateInfo, &materialMap);

	SgShader pShadersLighting[] = {lightingVertShader, lightingFragShader};
	SgMaterialCreateInfo materialLightingCreateInfo = {
		.pMaterialName = "materialLighting",
		.pShaders = pShadersLighting,
		.shaderCount = NUMOF(pShadersLighting),
		.resourceBindingCount = NUMOF(materialLightingSetResourceBinding),
		.pResourceBindings = materialLightingSetResourceBinding,
		.renderObjectCount = 1,
	};
	sgAddMaterial(&materialLightingCreateInfo, &materialMap);
	sgInitMaterialMap(app, &materialMap);

	//

	SgRenderObject pMyriamRenderObjects[] = {
		{
			.meshID = myriamMeshID,
			.instanceCount = NUMOF(myriamObjects),
		},
	};
	SgResourceCreateInfo myriamTransformMatricesCreateInfo = {
		.bytes = myriamObjects,
		.size = sizeof(myriamObjects),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "myriamTransformMatrices",
	};
	sgAddResource(app, &myriamTransformMatricesCreateInfo, &resourceMap);

	SgResourceCreateInfo lightingResourceCreateInfo = {
		.bytes = &lighting,
		.size = sizeof(lighting),
		.type = SG_RESOURCE_TYPE_UNIFORM,
		.pName = "lighting",
	};
	sgAddResource(app, &lightingResourceCreateInfo, &resourceMap);

	const char* pMyriamRenderObjectResourceNames[] = {
		meshResourceCreateInfo.pName,
		/*TODO: texture arrays */
		bodyDTextureResourceCreateInfo.pName,
		normalTextureResourceCreateInfo.pName,
		/**/
		myriamTransformMatricesCreateInfo.pName,
		lightingResourceCreateInfo.pName,
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
		meshResourceCreateInfo.pName,
		meshTextureResourceCreateInfo.pName,
		chaletTransformMatricesCreateInfo.pName,
		lightingResourceCreateInfo.pName,
		cameraResourceCreateInfo.pName
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
	SgResourceCreateInfo kittenTransformMatricesCreateInfo = {
		.bytes = kittenObjects,
		.size = sizeof(kittenObjects),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "kittenTransformMatrices",
	};
	sgAddResource(app, &kittenTransformMatricesCreateInfo, &resourceMap);

	SgResourceCreateInfo sphereTransformMatricesCreateInfo = {
		.bytes = sphereObjects,
		.size = sizeof(sphereObjects),
		.type = SG_RESOURCE_TYPE_MESH,

		.pName = "sphereTransformMatrices",
	};
	sgAddResource(app, &sphereTransformMatricesCreateInfo, &resourceMap);
	sgInitResourceMap(app, &resourceMap);
	sgUpdateAllResources(app, resourceMap);
	
	const char* materialKittenResourceNames[] = {
		meshResourceCreateInfo.pName,
		meshTextureResourceCreateInfo.pName,
		kittenTransformMatricesCreateInfo.pName,
		lightingResourceCreateInfo.pName,
		cameraResourceCreateInfo.pName
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

	const char* materialSphereResourceNames[] = {
		meshResourceCreateInfo.pName,
		sphereTransformMatricesCreateInfo.pName,
		lightingResourceCreateInfo.pName,
		cameraResourceCreateInfo.pName
	};

	SgRenderObject sphereRenderObjects[] = {
		{
			.meshID = sphereMeshID,
			.instanceCount = NUMOF(sphereObjects),
		},
	};
	SgRenderObjectCreateInfo sphereRenderObject = {
		.materialName = "materialLighting",
		.pName = "sphereMesh",
		.pRenderObjects = sphereRenderObjects,
		.renderObjectCount = NUMOF(sphereRenderObjects),
		.ppResourceNames = materialSphereResourceNames,
		.resourceCount = NUMOF(materialSphereResourceNames),
	};
	sgAddMaterialRenderObjects(&sphereRenderObject, &materialMap);

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
		SgData lightingData = {
			.bytes = &lighting,
			.size  = sizeof(lighting),
		};
		sphereObjects[0].position[0] = lighting.position[0];
		sphereObjects[0].position[1] = lighting.position[1];
		sphereObjects[0].position[2] = lighting.position[2];
		sphereObjects[0].position[3] = lighting.position[3];
		SgData sphereData = {
			.bytes = &sphereObjects,
			.size  = sizeof(sphereObjects),
		};
		sgUpdateResource(app, resourceMap, &cameraData, cameraResourceCreateInfo.pName);
		const char* resourceNames[] = {
			cameraResourceCreateInfo.pName,
			sphereTransformMatricesCreateInfo.pName,
			lightingResourceCreateInfo.pName,
			
		};
		const SgData* resourceDatas[] = {
			&cameraData,
			&sphereData,
			&lightingData,
		};
		sgUpdateResources(app, resourceMap, NUMOF(resourceNames), resourceDatas, resourceNames);
	}

	sgUnloadTexture(&pChaletTexture);
	sgUnloadTexture(&pNormalTexture);
	sgUnloadTexture(&pBodyDTexture);
	sgDeinitUpdateCommands(app, &updateCommands);

	sgDestroyResourceMap(app, &resourceMap);
	sgDestroyMaterialMap(app, &materialMap);
	sgDestroyShader(app, &chaletVertShader);
	sgDestroyShader(app, &chaletFragShader);
	sgDestroyShader(app, &myriamVertShader);
	sgDestroyShader(app, &myriamFragShader);
	sgDestroyShader(app, &lightingVertShader);
	sgDestroyShader(app, &lightingFragShader);
	sgUnloadContexts(app, &contexts); 
	sgDestroyApp(&app);
	sgUnloadMesh(&pMesh);

	return 0;
}
