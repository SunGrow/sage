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

	/* Placeholder Mesh */
	SgMesh *pMesh;
	
	SgLoadMesh("res/kitten.obj", &pMesh);
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
	sgUpdateResource(app, &meshIndicesResource);

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
	    .pos = {-3.0f, -1.0f, -1.0f},
	    .front = {2.0f, 0.0f, 0.0f},
	    .up = {0.0f, 1.0f, 0.0f},
	    .speed = 1,
	    .sens = 6,
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
		.binding = 1,
		.bytes = &transformuniform,
		.size = sizeof(TransformUniform),
		.stage = SG_SHADER_STAGE_VERTEX_BIT,
		.type = SG_RESOURCE_TYPE_DYNAMIC,
	};
	sgCreateResource(app, &cameraResourceCreateInfo, &cameraResource);


	/* Resource Set Init */
	SgResource pSetResources[] = {meshResource, cameraResource};
	SgResourceSetCreateInfo resourceSetCreateInfo = {
		.pResources = pSetResources,
		.resourceCount = sizeof(pSetResources) / sizeof(pSetResources[0]),
		.setIndex = 0,
	};
	SgResourceSet resourceSet;
	sgCreateResourceSet(app, &resourceSetCreateInfo, &resourceSet);

	/* Graphics Instance Init */
	SgShader pShaders[] = {vertShader, fragShader};
	SgResourceSet pResourceSets[] = {resourceSet};
	SgGraphicsInstanceCreateInfo graphicsInstanceCreateInfo = {
		.pShaders = pShaders,
		.shaderCount = sizeof(pShaders) / sizeof(pShaders[0]),
		.pSets = pResourceSets,
		.setCount = sizeof(pResourceSets) / sizeof(pResourceSets[0]),
	};
	SgGraphicsInstance graphicsInstance;
	sgCreateGraphicsInstance(app, &graphicsInstanceCreateInfo, &graphicsInstance);
	sgUpdateResource(app, &meshResource);
	SgResourceSetInitInfo initInfo = {
		.resourceSet = resourceSet,
		.graphicsInstance = graphicsInstance,
		.pResources = pSetResources,
		.resourceCount = 2,
	};
	sgInitResourceSet(app, &initInfo);
	

	SgAppUpdateInfo updateInfo = {
		.app = app,
		.graphicsInstance = graphicsInstance,
		.pIndexResouces = &meshIndicesResource,
		.indexResourceCount = 1,
	};

	while(sgAppUpdate(&updateInfo)) {

	}

	sgDestroyApp(&app);

	return 0;
}
