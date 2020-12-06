#include "sage.h"
#include "sage_math.h"
#include "sage_rend.h"
#include "sage_res.h"
#include "log.h"

int main() {
	SgAppCreateInfo createInfo = {
		.pName = "Space Invaders",
		.size  = {640, 480},
//		.flags = SG_APP_WINDOW_FULLSCREEN,
	};

	SgApp app;

	sgCreateApp(&createInfo, &app);

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
	typedef struct Camera_T {
		v3 pos;
		v3 front;
		v3 up;
		float speed;
		float sens;
		double deltatime;
	} Camera;

	Camera camera = {
	    .pos = {-2.0f, -1.0f, -1.0f},
	    .front = {2.0f, 0.0f, 0.0f},
	    .up = {0.0f, 1.0f, 0.0f},
	    .speed = 1,
	    .sens = 6,
		.deltatime = 0.003,
	};
	typedef struct TransformUniform_T {
		m4 model;
		m4 view;
		m4 proj;
	} TransformUniform;

	TransformUniform transformuniform = {0};
	for (uint32_t i = 0; i < 4; ++i) {
		for (uint32_t j = 0; j < 4; ++j) {
			if (i == j)
				transformuniform.model[i][j] = 1;
			else
				transformuniform.model[i][j] = 0;
		}
	}

	v3 at;
	v3_add(at, camera.pos, camera.front);
	lookat(transformuniform.view, camera.pos, at, camera.up);
	perspective(transformuniform.proj, deg_to_rad(80.0f),
	            createInfo.size[0] / (float)createInfo.size[1], 0.1f, 10.0f);
	/**/
	SgResource cameraResource;
	SgResourceCreateInfo cameraResourceCreateInfo = {
		.binding = 0,
		.bytes = &transformuniform,
		.size = sizeof(TransformUniform),
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

	while(sgAppUpdate(&updateInfo)) {
		v3 tmp;
		v3 right;
		v3_cross(right, camera.front, camera.up);
		v3_normalize_to(right, right);

		v3_scale_by(tmp, camera.speed * camera.deltatime * 1, camera.front);
		v3_add(camera.pos, tmp, camera.pos);
		v3_scale_by(tmp, camera.speed * camera.deltatime * 1, camera.up);
		v3_add(camera.pos, tmp, camera.pos);
		v3_scale_by(tmp, camera.speed * camera.deltatime * 1, right);
		v3_add(camera.pos, tmp, camera.pos);

		v3 at;
		v3_add(at, camera.pos, camera.front);
		lookat(transformuniform.view, camera.pos, at, camera.up);
		SgData cameraData = {
			.bytes = &transformuniform,
			.size = sizeof(transformuniform),
		};
		sgUpdateResource(app, &cameraData, &cameraResource);
	}

	sgDestroyApp(&app);

	return 0;
}
