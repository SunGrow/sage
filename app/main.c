#include <math.h>
#include "inputFuncs.h"
#include "inputKeys.h"
#include "resourceBindings.h"
#include "sage.h"
#include "transformObjects.h"

#define NUMOF(arr) (sizeof(arr) / sizeof((arr)[0]))

int main() {
	// Testing

	SgFile      contextConfigFile;
	const char* contextConfigFilePath = "cfg/contexts.json";
	sgOpenFile(contextConfigFilePath, &contextConfigFile);

	SgInputAction inputActions[] = {
	    keyPressForward,
	    keyPressLeft,
	    keyPressBack,
	    keyPressRight,
	    keyPressUp,
	    keyPressDown,
	    keyShoot,

	    rotateCamera,

	    lightKeyPressForward,
	    lightKeyPressUp,
	    lightKeyPressLeft,
	    lightKeyPressRight,
	    lightKeyPressDown,
	    lightKeyPressBack,
	};
	char* inputActionNames[] = {
	    "keyPressForward",
	    "keyPressLeft",
	    "keyPressBack",
	    "keyPressRight",
	    "keyPressUp",
	    "keyPressDown",
	    "keyShoot",

	    "rotateCamera",

	    "lightKeyPressForward",
	    "lightKeyPressUp",
	    "lightKeyPressLeft",
	    "lightKeyPressRight",
	    "lightKeyPressDown",
	    "lightKeyPressBack",
	};

	struct SgLighting {
		v4 position;
		v4 color;
	} lighting = {
	    .position = {0, 0, 0.0, 1.0},
	    .color    = {1.0, 1.0, 0.6, 1.0},
	};

	SgTmpCamera camera = {
	    .position    = {-2.0f, 1.0f, -1.0f},
	    .front       = {2.0f, 0.0f, 0.0f},
	    .up          = {0.0f, 1.0f, 0.0f},
	    .speed       = 1,
	    .sensitivity = 6,
	};

	SgCameraTransformInfo cameraTransform = {0};
	cameraTransform.camera                = camera;

	SgTimedCamera timedCamera = {0};

	SgActor testActors[] = {
	    &cameraTransform,
	    &lighting,
	};

	char* actorNames[] = {
	    "cameraTransform",
	    "lighting",
	};

	SgActiveContexts           contexts;
	SgActiveContextsCreateInfo activeContextsCreateInfo = {

	    .pActionFuncs = inputActions,
	    .pActionNames = inputActionNames,
	    .actionCount  = NUMOF(inputActions),

	    .pActors     = testActors,
	    .pActorNames = actorNames,
	    .actorCount  = NUMOF(testActors),

	    .pInputSignals = inputSignals,
	    .signalCount   = NUMOF(inputSignals),

	    .file = contextConfigFile,
	};

	sgLoadContexts(&activeContextsCreateInfo, &contexts);
	sgCloseFile(&contextConfigFile);

	SgFile      appConfigFile;
	const char* appConfigFilePath = "cfg/app.json";
	sgOpenFile(appConfigFilePath, &appConfigFile);
	SgAppCreateInfo createInfo = {
	    .pName      = "Space Invaders",
	    .configFile = appConfigFile,
	};
	SgApp app;
	sgCreateApp(&createInfo, &app);

	sgCloseFile(&appConfigFile);

	cameraTransform.camera.aspectRatio = 16 / 9;
	cameraTransform.camera.fov         = deg_to_rad(80.f);
	sgTimedCameraInit(&timedCamera);

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
	    .file  = chaletVertShaderFile,
	    .stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo chaletFragShaderCreateInfo = {
	    .file  = chaletFragShaderFile,
	    .stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShaderCreateInfo myriamVertShaderCreateInfo = {
	    .file  = myriamVertShaderFile,
	    .stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo myriamFragShaderCreateInfo = {
	    .file  = myriamFragShaderFile,
	    .stage = SG_SHADER_STAGE_FRAGMENT_BIT,
	};
	SgShaderCreateInfo lightingVertShaderCreateInfo = {
	    .file  = lightingVertShaderFile,
	    .stage = SG_SHADER_STAGE_VERTEX_BIT,
	};
	SgShaderCreateInfo lightingFragShaderCreateInfo = {
	    .file  = lightingFragShaderFile,
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
	    .size  = sizeof(SgTransformUniform),
	    .type  = SG_RESOURCE_TYPE_UNIFORM,
	    .pName = "cameraResource",
	};
	sgAddResource(app, &cameraResourceCreateInfo, &resourceMap);

	/* Placeholder Mesh */
	SgMeshSet* pMesh;

	sgCreateMeshSet(&pMesh);
	SgSize kittenMeshID      = sgAddMesh("res/kitten.obj", &pMesh);
	SgSize chaletMeshID      = sgAddMesh("res/chalet.obj", &pMesh);
	SgSize myriamMeshID      = sgAddMesh("res/myriam.obj", &pMesh);
	SgSize lightSourceMeshID = sgAddMesh("res/sphere.obj", &pMesh);

	/* Resource Init */
	SgResourceCreateInfo meshResourceCreateInfo = {
	    .bytes = pMesh->pVertices,
	    .size  = pMesh->vertexCount * sizeof(pMesh->pVertices[0]),
	    .type  = SG_RESOURCE_TYPE_MESH,

	    .pName = "meshResource"};
	sgAddResource(app, &meshResourceCreateInfo, &resourceMap);

	SgResourceCreateInfo meshIndicesResourceCreateInfo = {
	    .bytes = pMesh->pIndices,
	    .size  = pMesh->indexCount * sizeof(pMesh->pIndices[0]),
	    .type  = SG_RESOURCE_TYPE_INDICES,

	    .pName = "meshIndicesResource",
	};
	sgAddResource(app, &meshIndicesResourceCreateInfo, &resourceMap);
	// TODO: Wrap into a creation inside of an api, ffs
	pMesh->indicesResourceName = meshIndicesResourceCreateInfo.pName;
	/* Placeholder Mesh2 */
	/**/

	/* Resource Init */
	SgTexture* pChaletTexture;
	SgTexture* pNormalTexture;
	SgTexture* pBodyDTexture;
	SgTexture* pDefTexture;
	sgLoadTexture("res/chalet.jpg", &pChaletTexture);
	sgLoadTexture("res/tex/10016_w_Myriam_Body_N_2k.jpg", &pNormalTexture);
	sgLoadTexture("res/tex/10016_w_Myriam_Body_D_2k.jpg", &pBodyDTexture);
	sgLoadTexture("res/def.jpg", &pDefTexture);

	SgResourceCreateInfo meshTextureResourceCreateInfo = {
	    .bytes      = pChaletTexture->pixels,
	    .size       = pChaletTexture->size,
	    .extent     = (VkExtent3D){.height = pChaletTexture->height,
                             .width  = pChaletTexture->width,
                             .depth  = 1.0},
	    .type       = SG_RESOURCE_TYPE_TEXTURE_2D,
	    .layerCount = 1,
	    .levelCount = 1,

	    .pName = "chaletTextureResource",
	};
	SgResourceCreateInfo normalTextureResourceCreateInfo = {
	    .bytes      = pNormalTexture->pixels,
	    .size       = pNormalTexture->size,
	    .extent     = (VkExtent3D){.height = pNormalTexture->height,
                             .width  = pNormalTexture->width,
                             .depth  = 1.0},
	    .type       = SG_RESOURCE_TYPE_TEXTURE_2D,
	    .layerCount = 1,
	    .levelCount = 1,

	    .pName = "myriamNormalTextureResource",
	};
	SgResourceCreateInfo bodyDTextureResourceCreateInfo = {
	    .bytes      = pBodyDTexture->pixels,
	    .size       = pBodyDTexture->size,
	    .extent     = (VkExtent3D){.height = pBodyDTexture->height,
                             .width  = pBodyDTexture->width,
                             .depth  = 1.0},
	    .type       = SG_RESOURCE_TYPE_TEXTURE_2D,
	    .layerCount = 1,
	    .levelCount = 1,

	    .pName = "myriamBodyDTextureResource",
	};
	SgResourceCreateInfo defTextureResourceCreateInfo = {
	    .bytes      = pDefTexture->pixels,
	    .size       = pDefTexture->size,
	    .extent     = (VkExtent3D){.height = pDefTexture->height,
                             .width  = pDefTexture->width,
                             .depth  = 1.0},
	    .type       = SG_RESOURCE_TYPE_TEXTURE_2D,
	    .layerCount = 1,
	    .levelCount = 1,

	    .pName = "DefTextureResource",
	};

	sgAddResource(app, &meshTextureResourceCreateInfo, &resourceMap);
	sgAddResource(app, &normalTextureResourceCreateInfo, &resourceMap);
	sgAddResource(app, &bodyDTextureResourceCreateInfo, &resourceMap);
	sgAddResource(app, &defTextureResourceCreateInfo, &resourceMap);
	///

	SgMaterialMap           materialMap;
	SgMaterialMapCreateInfo materialMapCreateInfo = {
	    .materailCount = 2,
	    .resourceMap   = resourceMap,
	};
	// Materials
	sgCreateMaterialMap(app, &materialMapCreateInfo, &materialMap);

	SgShader pShadersChalet[]   = {chaletVertShader, chaletFragShader};
	SgShader pShadersMyriam[]   = {myriamVertShader, myriamFragShader};
	SgShader pShadersLighting[] = {lightingVertShader, lightingFragShader};

	SgMaterialCreateInfo materialChaletCreateInfo = {
	    .pMaterialName        = "materialChalet",
	    .pShaders             = pShadersChalet,
	    .shaderCount          = NUMOF(pShadersChalet),
	    .resourceBindingCount = NUMOF(materialChaletResourceBindings),
	    .pResourceBindings    = materialChaletResourceBindings,
	    .renderObjectCount    = 2,
	};

	SgMaterialCreateInfo materialMyriamCreateInfo = {
	    .pMaterialName        = "materialMyriam",
	    .pShaders             = pShadersMyriam,
	    .shaderCount          = NUMOF(pShadersMyriam),
	    .resourceBindingCount = NUMOF(materialMyriamSetResourceBinding),
	    .pResourceBindings    = materialMyriamSetResourceBinding,
	    .renderObjectCount    = 1,
	};

	SgMaterialCreateInfo materialLightingCreateInfo = {
	    .pMaterialName        = "materialLighting",
	    .pShaders             = pShadersLighting,
	    .shaderCount          = NUMOF(pShadersLighting),
	    .resourceBindingCount = NUMOF(materialLightingSetResourceBinding),
	    .pResourceBindings    = materialLightingSetResourceBinding,
	    .renderObjectCount    = 1,
	};

	sgAddMaterial(&materialChaletCreateInfo, &materialMap);
	sgAddMaterial(&materialMyriamCreateInfo, &materialMap);
	sgAddMaterial(&materialLightingCreateInfo, &materialMap);

	sgInitMaterialMap(app, &materialMap);
	//

	SgRenderObject pMyriamRenderObjects[] = {
	    {
	        .meshID        = myriamMeshID,
	        .instanceCount = NUMOF(myriamObjects),
	    },
	};
	SgResourceCreateInfo myriamTransformMatricesCreateInfo = {
	    .bytes = myriamObjects,
	    .size  = sizeof(myriamObjects),
	    .type  = SG_RESOURCE_TYPE_MESH,

	    .pName = "myriamTransformMatrices",
	};
	sgAddResource(app, &myriamTransformMatricesCreateInfo, &resourceMap);

	SgResourceCreateInfo lightingResourceCreateInfo = {
	    .bytes = &lighting,
	    .size  = sizeof(lighting),
	    .type  = SG_RESOURCE_TYPE_UNIFORM,
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
	    .materialName      = "materialMyriam",
	    .pName             = "myriamMesh",
	    .pRenderObjects    = pMyriamRenderObjects,
	    .renderObjectCount = NUMOF(pMyriamRenderObjects),
	    .ppResourceNames   = pMyriamRenderObjectResourceNames,
	    .resourceCount     = NUMOF(pMyriamRenderObjectResourceNames),
	};
	sgAddMaterialRenderObjects(&myriamRenderObject, &materialMap);

	SgRenderObject chaletRenderObjects[] = {
	    {
	        .meshID        = chaletMeshID,
	        .instanceCount = NUMOF(chaletObjects),
	    },
	};

	SgResourceCreateInfo chaletTransformMatricesCreateInfo = {
	    .bytes = chaletObjects,
	    .size  = sizeof(chaletObjects),
	    .type  = SG_RESOURCE_TYPE_MESH,

	    .pName = "chaletTransformMatrices",
	};
	sgAddResource(app, &chaletTransformMatricesCreateInfo, &resourceMap);
	const char* materialChaletResourceNames[] = {
	    meshResourceCreateInfo.pName, meshTextureResourceCreateInfo.pName,
	    chaletTransformMatricesCreateInfo.pName, lightingResourceCreateInfo.pName,
	    cameraResourceCreateInfo.pName};
	SgRenderObjectCreateInfo chaletRenderObject = {
	    .materialName      = "materialChalet",
	    .pName             = "chaletMesh",
	    .pRenderObjects    = chaletRenderObjects,
	    .renderObjectCount = NUMOF(chaletRenderObjects),
	    .ppResourceNames   = materialChaletResourceNames,
	    .resourceCount     = NUMOF(materialChaletResourceNames),
	};

	sgAddMaterialRenderObjects(&chaletRenderObject, &materialMap);
	SgResourceCreateInfo kittenTransformMatricesCreateInfo = {
	    .bytes = kittenObjects,
	    .size  = sizeof(kittenObjects),
	    .type  = SG_RESOURCE_TYPE_MESH,

	    .pName = "kittenTransformMatrices",
	};
	sgAddResource(app, &kittenTransformMatricesCreateInfo, &resourceMap);

	SgResourceCreateInfo sphereTransformMatricesCreateInfo = {
	    .bytes = lightingObjects,
	    .size  = sizeof(lightingObjects),
	    .type  = SG_RESOURCE_TYPE_MESH,

	    .pName = "lightingTransformMatrices",
	};
	sgAddResource(app, &sphereTransformMatricesCreateInfo, &resourceMap);
	sgInitResourceMap(app, &resourceMap);
	sgUpdateAllResources(app, resourceMap);

	const char* materialKittenResourceNames[] = {
	    meshResourceCreateInfo.pName, meshTextureResourceCreateInfo.pName,
	    kittenTransformMatricesCreateInfo.pName, lightingResourceCreateInfo.pName,
	    cameraResourceCreateInfo.pName};
	SgRenderObject kittenRenderObjects[] = {
	    {
	        .meshID        = kittenMeshID,
	        .instanceCount = NUMOF(kittenObjects),
	    },
	};
	SgRenderObjectCreateInfo kittenRenderObject = {
	    .materialName      = "materialChalet",
	    .pName             = "kittenMesh",
	    .pRenderObjects    = kittenRenderObjects,
	    .renderObjectCount = NUMOF(kittenRenderObjects),
	    .ppResourceNames   = materialKittenResourceNames,
	    .resourceCount     = NUMOF(materialKittenResourceNames),
	};
	sgAddMaterialRenderObjects(&kittenRenderObject, &materialMap);

	const char* materialLightSourceResourceNames[] = {
	    meshResourceCreateInfo.pName, sphereTransformMatricesCreateInfo.pName,
	    lightingResourceCreateInfo.pName, cameraResourceCreateInfo.pName};

	SgRenderObject lightSourceRenderObjects[] = {
	    {
	        .meshID        = lightSourceMeshID,
	        .instanceCount = NUMOF(lightingObjects),
	    },
	};
	SgRenderObjectCreateInfo sphereRenderObject = {
	    .materialName      = "materialLighting",
	    .pName             = "lightSourceMesh",
	    .pRenderObjects    = lightSourceRenderObjects,
	    .renderObjectCount = NUMOF(lightSourceRenderObjects),
	    .ppResourceNames   = materialLightSourceResourceNames,
	    .resourceCount     = NUMOF(materialLightSourceResourceNames),
	};
	sgAddMaterialRenderObjects(&sphereRenderObject, &materialMap);

	sgWriteMaterialRenderObjects(&materialMap);

	/* Graphics Instance Init */

	SgUpdateCommandsInitInfo updateInitInfo = {
	    .materialMap = materialMap,
	    .pMeshSet    = pMesh,
	    .resourceMap = resourceMap,
	};
	SgUpdateCommands updateCommands;
	sgInitUpdateCommands(&updateInitInfo, &updateCommands);

	SgAppUpdateInfo updateInfo = {
	    .app            = app,
	    .materialMap    = materialMap,
	    .pMeshSet       = pMesh,
	    .updateCommands = updateCommands,
	};
	sgSetActiveContexts(contexts, &app);

	while (sgAppUpdate(&updateInfo)) {
		sgTimedCameraUpdate(&timedCamera);
		cameraTransform.deltaTime = timedCamera.deltaTime;
		sgTransformCamera(&cameraTransform, &cameraTransform.camera);
		sgUpdateTransformUniform(&cameraTransform.camera, &transformuniform);
		cameraTransform.cursorOffset[0] = 0;
		cameraTransform.cursorOffset[1] = 0;
		SgData cameraData               = {
        .bytes = &transformuniform,
        .size  = sizeof(transformuniform),
    };
		SgData lightingData = {
		    .bytes = &lighting,
		    .size  = sizeof(lighting),
		};
		lightingObjects[0].position[0] = lighting.position[0];
		lightingObjects[0].position[1] = lighting.position[1];
		lightingObjects[0].position[2] = lighting.position[2];
		lightingObjects[0].position[3] = lighting.position[3];
		SgData sphereData              = {
        .bytes = &lightingObjects,
        .size  = sizeof(lightingObjects),
    };
		sgUpdateResource(app, resourceMap, &cameraData,
		                 cameraResourceCreateInfo.pName);
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
		sgUpdateResources(app, resourceMap, NUMOF(resourceNames), resourceDatas,
		                  resourceNames);
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
