#ifndef SG_MESH
#define SG_MESH

#include "sage_base.h"
#include "sg_math.h"
#include "stb_image.h"

// Mesh
typedef struct SgVertex {
	float vx, vy, vz;
	float nx, ny, nz;
	float tu, tv;
} SgVertex;

typedef struct SgMesh {
	SgVertex   *pVertices;
	uint32_t   vertexCount;
	uint32_t   *pIndices;
	uint32_t   indexCount;
} SgMesh;
//


// Mesh
void sgLoadMesh(const char *pPath, SgMesh **ppMesh);
void sgUnloadMesh(SgMesh **ppMesh);

typedef struct SgMeshTransformInfo {
	v3 move;
	v3 scale;
} SgMeshTransformInfo;

void sgTransformMesh(const SgMeshTransformInfo *pTransformInfo, uint32_t vertCount, SgVertex *pVertices);

typedef struct SgTexture {
	int32_t width, height, channels;
	stbi_uc *pixels;
} SgTexture;

void sgLoadTexture(const char *pPath, SgTexture **ppTexture);
void sgUnloadTexture(SgTexture **ppTexture);

#endif
