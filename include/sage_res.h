#ifndef SAGE_RES_H_
#define SAGE_RES_H_ 1

#include "sage_core.h"
#include "sage_math.h"
#include "stb_image.h"

/* Make mesh load fit with the engine theme */
typedef struct SgVertex {
	v3 vert;
	v3 norm;
	v2 tex;
} SgVertex;

typedef struct SgMesh {
	SgVertex   *pVertices;
	uint32_t   vertexCount;
	uint32_t   *pIndices;
	uint32_t   indexCount;
} SgMesh;

void sgLoadMesh(const char *pPath, SgMesh **ppMesh);
void sgUnloadMesh(SgMesh **ppMesh);

typedef struct SgMeshTransformInfo {
	v3 move;
	v3 scale;
} SgMeshTransformInfo;

typedef struct SgTexture {
	int32_t width, height, channels;
	uint32_t size;
	stbi_uc *pixels;
} SgTexture;

void sgLoadTexture(const char *pPath, SgTexture **pTexture);
void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t vertCount, SgVertex *pVertices);
void sgUnloadTexture(SgTexture **ppTexture);

#endif
